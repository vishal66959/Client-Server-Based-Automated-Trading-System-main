

/*

Five Traders Login Credentials

Username     Password
    t1          p1
    t2          p2
    t3          p3
    t4          p4
    t5          p5

*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char input[1024];
char buffer[1024];
int check;

void login(int clientSocket) {

    printf("Enter UserName: ");
    scanf("%s",input);
    check = send(clientSocket, input, strlen(input), 0);
    if (check < 0) {
        printf("Error Sending Username!!!");
        exit(1);
    }
    
    bzero(input, 1024);
    bzero(buffer, sizeof(buffer));
    check = read(clientSocket, buffer, 1024);
    if(check < 0){
        printf("Error reading username confirmation message Socket\n");
        close(clientSocket);
        exit(1);
    }

    if(strcmp(buffer, "12") == 0) {
        printf("Wrong Username\n");
        close(clientSocket);
        exit(1);
    }
    
    printf("Enter Password: ");
    scanf("%s",input);
    
    check = send(clientSocket, input, strlen(input), 0);
    
    if(check < 0){
        printf("Error Sending Paasword\n");
        close(clientSocket);
        exit(1);
    }
    bzero(input, 1024);
    bzero(buffer, sizeof(buffer));
    
    check = read(clientSocket, buffer, 1024);
    if(check < 0){
        printf("Error Reading password confirmation message\n");
        close(clientSocket);
        exit(1);
    }

    if(strcmp(buffer, "12") == 0) {
        printf("Wrong Password\n");
        close(clientSocket);
        exit(1);
    }

    bzero(buffer, sizeof(buffer));
    
    check = read(clientSocket, buffer, 1024);
    if(check < 0){
        printf("Error Reading Already Logged In Message\n");
        close(clientSocket);
        exit(1);
    }

    if(strcmp(buffer, "12") == 0) {
        printf("Already Logged In on Another Client\n");
        close(clientSocket);
        exit(1);
    }
    printf("\nYou are Logged In...\n ");

}

void sendBuyRequest(int clientSocket) {

    int itemCode, itemQuantity, itemPrice;

    printf("Enter Item Code (in range 1 to 10), Quantity(>0) and Price(>0) :\n ");
    scanf("%d %d %d", &itemCode, &itemQuantity, &itemPrice);

    if(itemCode >= 1 && itemCode <= 10 && itemQuantity >= 1 && itemPrice > 0) {

        sprintf(input,"%d %d %d %d", 1, itemCode, itemQuantity, itemPrice);
        check = send(clientSocket, input, strlen(input), 0);
        if(check < 0){
            printf("Error Sending Buy Request\n");
            close(clientSocket);
            exit(1);
        }
    }
    else {
       printf("Wrong Data Given\n\n");
    }
}

void sendSellRequest(int clientSocket) {

    int itemCode, itemQuantity, itemPrice;

    printf("Enter Item Code (in range 1 to 10), Quantity(>0) and Price(>0) :\n ");
    scanf("%d %d %d", &itemCode, &itemQuantity, &itemPrice);

    if(itemCode >= 1 && itemCode <= 10 && itemQuantity >= 1 && itemPrice > 0) {

        sprintf(input,"%d %d %d %d", 2, itemCode, itemQuantity, itemPrice);
        check = send(clientSocket, input, strlen(input), 0);
        if(check < 0){
            printf("Error Sending Sell Request\n");
            close(clientSocket);
            exit(1);
        }
    }
    else {
       printf("Wrong Data Given\n\n");
    }
}

void viewOrderStatus(int clientSocket) {

    check = send(clientSocket, "3", 1, 0);
    if(check < 0){
        printf("Error Sending View Order Status Request\n");
        close(clientSocket);
        exit(1);
    }
    bzero(buffer,1024);
    check = read(clientSocket, buffer, 1024);
    if(check < 0){
        printf("Error Reading View Trade Status Request\n");
        close(clientSocket);
        exit(1);
    }
    printf("\n%s\n", buffer);
}

void viewTradeStatus(int clientSocket) {

    check = send(clientSocket, "4", 1, 0);
    if(check < 0){
        printf("Error Sending View Trade Status Request\n");
        close(clientSocket);
        exit(1);
    }
    bzero(buffer,1024);
    check = read(clientSocket, buffer, 1024);
    if(check < 0){
        printf("Error Reading View Order Status Request\n");
        close(clientSocket);
        exit(1);
    }
    printf("\n%s\n", buffer);
}

int main(int argc, char *argv[]){

    if(argc < 2) {
        printf("Port Not Provided\n");
        exit(1);
    }

    int clientSocket, portNo = atoi(argv[2]);
    struct sockaddr_in serverAddress;
    struct hostent *server;

    server = gethostbyname(argv[1]);
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portNo);
    

    // Socket creation
    check = clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(check < 0){
        printf("Error Creating Socket\n");
        exit(1);
    }

    //Connecting with Server
    check = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if(check < 0){
        printf("Error Connecting to Server\n");
        close(clientSocket);
        exit(1);
    }

    //Logging in trader on client
    login(clientSocket);

    //Connected
    printf("* Connection established with Server.\n");
    printf("* To exit type 0\n\n");

    while(1){

        printf("1.Send Buy Request\n");
        printf("2.Send Sell Request\n");
        printf("3.View Order Status\n");
        printf("4.View Trade Status\n");
        printf("Choose an Option\n");

        int option;
        scanf("%d", &option);

        switch(option) {
            case 0:
                check = send(clientSocket, "exit", 4, 0);
                if(check < 0){
                    printf("Error in sending exit message\n");
                    close(clientSocket);
                    exit(1);
                }
                close(clientSocket);
                exit(1);
            case 1:
                sendBuyRequest(clientSocket);
                break;
            case 2:
                sendSellRequest(clientSocket);
                break;

            case 3:
                viewOrderStatus(clientSocket);
                break;

            case 4:
                viewTradeStatus(clientSocket);
                break;
            default:
                printf("Choose correct option or 0 to exit...\n");
        }
    } 
    close(clientSocket);
    return 0;
}



/*

ID      Password
t1          p1
t2          p2
t3          p3
t4          p4
t5          p5

*/