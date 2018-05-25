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
    char *message=malloc(sizeof(char)*1024);
    //char message;//[256]={};
    char receiveMessage[1024];
    //char * information=malloc(sizeof(char)*256);
    char ready='i';
//    int size;
    while(1) {
        // char* receiveMessage=malloc(sizeof(char)*1024);
//        memset(receiveMessage,'\0',sizeof(receiveMessage));
//      recv(sockfd,&size,1,0);

        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);//先接到要求的訊息
        printf("%s",receiveMessage);

        scanf(" %s",message);
        send(sockfd,message,sizeof(message),0);//送出request

        if(strcmp(message,"a")==0) {//如果自己送出的request==a
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);//接收add what word?
            printf("%s\n",receiveMessage);
            scanf(" %s",message);
            send(sockfd,message,sizeof(message),0);//送出要add什麼字
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);//接收是否add成功
            printf("%s\n",receiveMessage);
        } else if (strcmp(message,"f")==0) {
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            printf("%s\n",receiveMessage);
            scanf(" %s",message);
            send(sockfd,message,sizeof(message),0);//送出要find什麼字
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);//接收find結果
            printf("%s\n",receiveMessage);
        } else if (strcmp(message,"s")==0) {
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            printf("%s\n",receiveMessage);
            scanf(" %s",message);
            send(sockfd,message,sizeof(message),0);//送出要find什麼字

            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            printf("%s\n",receiveMessage);
        }
        /*
        else if (strcmp(message,"d")==0) {
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            printf("%s\n",receiveMessage);
        } */
        else if (strcmp(message,"q")==0) {
            close(sockfd);
            exit(0);
        }
        //送出一個dummy的訊息
        ready='i';
        send(sockfd,&ready,sizeof(ready),0);
    }
    return 0;
}
