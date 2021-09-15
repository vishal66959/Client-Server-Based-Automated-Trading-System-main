

/*
									Vikash Raghuwanshi 204101059
									Vivek Singh		   204101061
									Utkarsh Khati	   204101058
									Vishal Jaiswal	   204101060
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

char buffer[1024];

char* traderLogIn[5] = {"login0.txt", "login1.txt", "login2.txt", "login3.txt", "login4.txt"};

char* itemBestBuySellValue[10] = {"items0.txt","items1.txt", "items2.txt", "items3.txt", "items4.txt", "items5.txt", "items6.txt", "items7.txt", "items8.txt", "items9.txt"};

//linked list
struct node {
	int traderId, itemQuantity, itemPrice;
	struct node* next;
};

struct queue {
	int itemCode;
	struct node *buyQueue, *sellQueue; 
};
struct queue requestQueues[10];

char* tradeSets[] = {"trade1.txt", "trade2.txt", "trade3.txt", "trade4.txt", "trade5.txt"};


struct node* createNode(int itemQuantity, int itemPrice,int traderId) {
	struct node * newNode = (struct node*) malloc(sizeof(struct node));
	newNode->traderId = traderId;
	newNode->itemQuantity = itemQuantity;
	newNode->itemPrice = itemPrice;
	newNode->next = NULL;
	return newNode;
}


void viewTradeStatus(int clientSocket, int traderId)
{
    printf("View Trade Status\n");

    FILE *fileRead = fopen(tradeSets[traderId - 1], "r");

    char *buffer = (char *)malloc(40 * sizeof(char));
    char *tradeStatus = (char *)malloc(1024 * sizeof(char));
    bzero(buffer, sizeof(buffer));

    int len = strlen("\nBuy/Sell ItemCode Quantity Price from/to-TraderId\n");

    bcopy("\nBuy/Sell ItemCode Quantity Price from/to-TraderId\n", tradeStatus, len);

    while (fgets(buffer, 39, fileRead) != NULL) {
        strcat(tradeStatus, buffer);
    }

    int n = send(clientSocket, tradeStatus, strlen(tradeStatus), 0);
    if (n < 0) {
        printf("Error Sending Trade Status!!!");
        exit(1);
    }
    fclose(fileRead);
}

void viewOrderStatus(int clientSocket) {

	printf("View Order Status\n");

	char* buffer = (char*)malloc(30*sizeof(char));
	char* orderStatus = (char*)malloc(1024*sizeof(char));
	strcpy(orderStatus,"");
	bzero(buffer, sizeof(buffer));

	for(int i=0; i<10; i++) {

		sprintf(buffer,"\nItem %d:\n",i+1);
		strcat(orderStatus,buffer);

		FILE *bestVal = fopen(itemBestBuySellValue[i], "r");
		int buy = 0, bQ = 0, sell = 0, sQ = 0, i = 0, num;
		fscanf(bestVal,"%d %d %d %d", &buy, &bQ, &sell, &sQ);
		fclose(bestVal);

		if(buy != 0) sprintf(buffer,"Best Buy Value: %d %d\n", buy, bQ);
		else sprintf(buffer, "Best Buy Value: N/A\n");
			
		strcat(orderStatus,buffer);

		if(sell != 0) sprintf(buffer,"Best Sell Value: %d %d\n", sell, sQ);
		else sprintf(buffer, "Best Sell Value: N/A\n");
		
		strcat(orderStatus,buffer);
	}
		
	int n = send(clientSocket, orderStatus, strlen(orderStatus), 0);
	if(n < 0) {
        printf("Error Sending Order Status!!!");
        exit(1);
    }

}

void appendBuyQueue(int itemCode, int itemQuantity, int itemPrice, int traderId) {

    struct node *newNode = createNode(itemQuantity, itemPrice, traderId);
    struct node *head = requestQueues[itemCode - 1].buyQueue;

    if (!head || head->itemPrice <= newNode->itemPrice) {
        newNode->next = head;
        requestQueues[itemCode-1].buyQueue = newNode;
    }
    else {
        while (head->next != NULL && head->next->itemPrice > newNode->itemPrice) {
            head = head->next;
        }
        newNode->next = head->next;
        head->next = newNode;
    }

}


void appendSellQueue(int itemCode,int itemQuantity,int itemPrice,int traderId){

    struct node* newNode = createNode(itemQuantity,itemPrice,traderId); 
    struct node *head = requestQueues[itemCode-1].sellQueue;

    if(!head || head->itemPrice > newNode->itemPrice){
    	newNode->next = head;
        requestQueues[itemCode-1].sellQueue = newNode;
    }
    else {
        while (head->next != NULL && head->next->itemPrice < newNode->itemPrice){
            head = head->next;
        }
        newNode->next = head->next;
        head->next = newNode;
    }

}

void buyRequest(int itemCode, int itemQuantity, int itemPrice, int traderId) {

    printf("Handle Buy Request\n");

    FILE *bestVal = fopen(itemBestBuySellValue[itemCode-1], "r");
	int buy = 0, bQ = 0, sell = 0, sQ = 0, i = 0, num;
	fscanf(bestVal,"%d %d %d %d", &buy, &bQ, &sell, &sQ);
	fclose(bestVal);
	bestVal = fopen(itemBestBuySellValue[i], "w");
	if(buy < itemPrice) {
        buy = itemPrice;
        bQ = itemQuantity;
    }

    fprintf(bestVal, "%d %d %d %d", buy, bQ, sell, sQ);
    fclose(bestVal);

    struct node *temp = requestQueues[itemCode - 1].sellQueue;
    FILE *buyer_fd = fopen(tradeSets[traderId - 1], "a");

    while (itemQuantity > 0) {
        temp = requestQueues[itemCode - 1].sellQueue;
        if (temp != NULL){
            if (temp->itemPrice <= itemPrice){
                FILE *seller_fd = fopen(tradeSets[temp->traderId - 1], "a");
                if (temp->itemQuantity == itemQuantity){

                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, itemQuantity, itemPrice, temp->traderId);
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, temp->itemQuantity, itemPrice, traderId);

                    itemQuantity = 0;
                    requestQueues[itemCode - 1].sellQueue = temp->next;
                    free(temp);
                }
                else if (temp->itemQuantity < itemQuantity) {
                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, temp->itemQuantity, itemPrice, temp->traderId);
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, temp->itemQuantity, itemPrice, traderId);

                    itemQuantity -= temp->itemQuantity;
                    requestQueues[itemCode - 1].sellQueue = temp->next;
                    free(temp);
                }
                else {
                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, itemQuantity, itemPrice, temp->traderId);
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, itemQuantity, itemPrice, traderId);

                    temp->itemQuantity -= itemQuantity;
                    itemQuantity = 0;
                }
                fclose(seller_fd);
                continue;
            }
            else{
                appendBuyQueue(itemCode, itemQuantity, itemPrice, traderId);
                break;
            }
        }
        else{
            appendBuyQueue(itemCode, itemQuantity, itemPrice, traderId);
            break;
        }
    }
    fclose(buyer_fd);
}

void sellRequest(int itemCode, int itemQuantity, int itemPrice, int traderId){

    printf("Handle Sell Request\n");

    FILE *bestVal = fopen(itemBestBuySellValue[itemCode-1], "r");
	int buy = 0, bQ = 0, sell = 0, sQ = 0, i = 0, num;
	fscanf(bestVal,"%d %d %d %d", &buy, &bQ, &sell, &sQ);
	fclose(bestVal);
	bestVal = fopen(itemBestBuySellValue[i], "w");

	if(sell == 0 || sell > itemPrice) {
	    sell = itemPrice;
	    sQ = itemQuantity;
	}

    fprintf(bestVal, "%d %d %d %d", buy, bQ, sell, sQ);
    fclose(bestVal);

    struct node *temp = requestQueues[itemCode - 1].buyQueue;
    FILE *seller_fd = fopen(tradeSets[traderId - 1], "a");
    while (itemQuantity > 0){
        temp = requestQueues[itemCode - 1].buyQueue;
        if (temp != NULL){
            if (temp->itemPrice >= itemPrice){
                FILE *buyer_fd = fopen(tradeSets[temp->traderId - 1], "a");

                if (temp->itemQuantity == itemQuantity) {
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, itemQuantity, itemPrice, temp->traderId);
                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, temp->itemQuantity, itemPrice, traderId);
                    itemQuantity = 0;
                    requestQueues[itemCode-1].buyQueue = temp->next;
                    free(temp);
                }
                else if (temp->itemQuantity < itemQuantity) {
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, temp->itemQuantity, itemPrice, temp->traderId);
                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, temp->itemQuantity, itemPrice, traderId);
                    itemQuantity -= temp->itemQuantity;
                    requestQueues[itemCode-1].buyQueue = temp->next;
                    free(temp);
                }
                else {
                    fprintf(seller_fd, "Sell item %d: Quantity: %d Price: %d to Trader%d\n", itemCode, itemQuantity, itemPrice, temp->traderId);
                    fprintf(buyer_fd, "Buy item %d: Quantity: %d Price: %d from Trader%d\n", itemCode, itemQuantity, itemPrice, traderId);
                    temp->itemQuantity -= itemQuantity;
                    itemQuantity = 0;
                }
                fclose(buyer_fd);
                continue;
            }
            else{
                appendSellQueue(itemCode, itemQuantity, itemPrice, traderId);
                break;
            }
        }
        else{
            appendSellQueue(itemCode, itemQuantity, itemPrice, traderId);
            break;
        }
    }
    fclose(seller_fd);
}



int main(int argc, char *argv[]){

    if(argc < 2) {
        printf("Port Not Provided\n");
        exit(1);
    }

	FILE* fileWrite = fopen(tradeSets[0],"w");
	fileWrite = fopen(tradeSets[1],"w");
	fileWrite = fopen(tradeSets[2],"w");
	fileWrite = fopen(tradeSets[3],"w");
	fileWrite = fopen(tradeSets[4],"w");

	int buy = 0, bQ = 0, sell = 0, sQ = 0;
	for(int i = 0; i < 10; i ++) {
		fileWrite = fopen(itemBestBuySellValue[i],"w");
	    fprintf(fileWrite, "%d %d %d %d", buy, bQ, sell, sQ);
	    fclose(fileWrite); 
	}

	int login = 0;
	for(int i = 0; i < 5; i ++) {
		fileWrite = fopen(traderLogIn[i],"w");
	    fprintf(fileWrite, "%d", login);
	    fclose(fileWrite); 
	}

	int serverSocket, portNo = atoi(argv[1]);
	struct sockaddr_in serverAddress;

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNo);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	int clientSocket;
	struct sockaddr_in tempAddress;

	socklen_t addressSize;

	pid_t childPid;

	//socket creation
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket < 0) {
        printf("Error Creating Socket!!!");
        exit(1);
    }
	printf("* Socket created Successfully...\n");

	// Binding with port nd checking the connection
	int check = bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
	if(check < 0){
		printf("* Error in binding\n");
		exit(1);
	}
	printf("* Binded Successfully...\n");

	//Listening
	listen(serverSocket, 5);
		printf("* Listening\n");

	int clientId = 0;

	while(1) {
		clientSocket = accept(serverSocket, (struct sockaddr*)&tempAddress, &addressSize);
		if(clientSocket < 0){
			printf("Error Accepting Trader!!!");
			continue;
		}

		bzero(buffer, sizeof(buffer));
		check = read(clientSocket, buffer, 1024);
		if(check < 0){
			printf("* Error in Reading UserName\n");
			continue;
		}
		printf("Client : %s\n", buffer);
		if(strcmp(buffer, "t1") == 0 || strcmp(buffer, "t2") == 0 || strcmp(buffer, "t3") == 0 || strcmp(buffer, "t4") == 0 || strcmp(buffer, "t5") == 0) {
			char pwd = buffer[1];
			check = send(clientSocket, "2", 1, 0);
			if(check < 0){
				printf("* Error in sending username confirmation...\n");
				continue;
			}
			bzero(buffer, sizeof(buffer));
			check = read(clientSocket, buffer, 1024);
			if(check < 0){
				printf("* Error in reading Password\n");
				continue;
			}
			printf("Client : %s\n", buffer);
			if((strcmp(buffer, "p1") == 0 || strcmp(buffer, "p2") == 0 || strcmp(buffer, "p3") == 0 || strcmp(buffer, "p4") == 0 || strcmp(buffer, "p5") == 0) && buffer[1] == pwd) {

				clientId = buffer[1] - 48;
				check = send(clientSocket, "2", 1, 0);
				if(check < 0){
					printf("* Error in sending password confirmation...\n");
					continue;
				}

				bzero(buffer, sizeof(buffer));

				FILE *fileWrite = fopen(traderLogIn[clientId-1], "r");
				int login;
				fscanf(fileWrite,"%d", &login);
				fclose(fileWrite);

				if(login == 0) {
				
					check = send(clientSocket, "2", 1, 0);
					if(check < 0){
						printf("* Error in sending already not logged in confirmation...\n");
						continue;
					}

					bzero(buffer, sizeof(buffer));

					login = 1;
					FILE *fileWrite = fopen(traderLogIn[clientId - 1], "w");
					fprintf(fileWrite, "%d", login);
					fclose(fileWrite);

					printf("Client %d is connected\n", clientId);

					if((childPid = fork()) != 0) {

						while(1){
							
							bzero(buffer, sizeof(buffer));
							check = read(clientSocket, buffer, 1024);
							if(check < 0){
								printf("* Error in reading Data\n");
								continue;
							}
							if(strcmp(buffer, "exit") == 0){
								printf("* Disconnected from Client %d\n", clientId);
								login = 0;
								FILE *fileWrite = fopen(traderLogIn[clientId - 1], "w");
								fprintf(fileWrite, "%d", login);
								fclose(fileWrite);
								break;
							}
							else {

								int option, itemCode, itemQuantity, itemPrice;
			                    buffer[254] = '\0';  
			                    switch(buffer[0]){
			                    	case '1':
			                    		sscanf(buffer,"%d %d %d %d", &option, &itemCode, &itemQuantity, &itemPrice);
			                    		buyRequest(itemCode, itemQuantity, itemPrice, clientId);		
			                    		break;
			                    	case '2':
			                    		sscanf(buffer,"%d %d %d %d",&option, &itemCode, &itemQuantity, &itemPrice);
			                    		sellRequest(itemCode, itemQuantity, itemPrice, clientId);
			                    		break;
			                    	case '3':
			                    		viewOrderStatus(clientSocket);
			                    		break;
			                		case '4':
			                			viewTradeStatus(clientSocket, clientId);
			                			break;
			                    	default:
			                    		break;	
			                    }
							}
						}
					// traderLogIn[clientId] = 0;
					}
					else if(childPid == -1) {
						printf("Fork Failed");
						continue;
					}

				}
				else {
					check = send(clientSocket, "12", 2, 0);
					if(check < 0){
						printf("* Error in sending already logged in confirmation...\n");
					}
					break;
				}
			}
			else {
				check = send(clientSocket, "12", 2, 0);
				if(check < 0){
					printf("* Error in sending password incorrect confirmation...\n");
				}
				break;
			}
		}
		else {
				check = send(clientSocket, "12", 2, 0);
				if(check < 0){
					printf("* Error in sending username incorrect confirmation...\n");
				}
				break;
			}
	}

	close(clientSocket);
	return 0;
}




