#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    //建立 socket
    int sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }
    //連線socket
    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(59487);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1) {
        printf("Connection error");
    }

    char message;//[256]={};
    char receiveMessage[512] = {};
    char information[256]= {};
    char ready='i';
    int size;
    while(1) {
        //char* receiveMessage=malloc(sizeof(char)*512);
        //memset(receiveMessage,'\0',sizeof(receiveMessage));
//      recv(sockfd,&size,1,0);
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);//先接到要求的訊息
        printf("%s",receiveMessage);

        scanf(" %c",&message);
        send(sockfd,&message,sizeof(message),0);//送出request
    }
exit:
    printf("close Socket.\n");
    close(sockfd);

    return 0;
}

