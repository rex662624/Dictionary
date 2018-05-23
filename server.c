#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

int sockfd = 0;
int num =0;//現在有幾個client
int *forClientSockfd;//動態配置,存放每個client的descriper
pthread_t *thread;

void *thread_function(void *);
void getprocessid(int []);
void getstat(char[],int,char);
void getchild(int[], int,char );
void getcmdline(char[], int );
void getthread(int[], int);
void getancient(int [],int,int);

int main(int argc, char **argv)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線
    //提供socket訊息
    struct sockaddr_in serverInfo,clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo,sizeof(serverInfo));//初始化
    serverInfo.sin_family = PF_INET;//sockaddr_in為Ipv4結構
    serverInfo.sin_addr.s_addr =  inet_addr("127.0.0.1");//IP
    serverInfo.sin_port = htons(59487);//port

    //綁定server在socket上
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    int threadid;
    while(1) {
        //監聽有沒有client來
        listen(sockfd,10000);
        //增加新的client,要把舊的data移到新allocate的地方
        int temp[num];
        pthread_t ptemp[num];
        int i;
        for(i=0; i<num; i++) {
            temp[i]=forClientSockfd[i];//先用temp存放剛剛的
            ptemp[i]=thread[i];
        }
        //重新allocate array(記得把num+1)
        forClientSockfd = malloc(sizeof(int)*(num+1));
        thread = malloc(sizeof(pthread_t)*(num+1));
        for(i=0; i<num; i++) {
            forClientSockfd[i]=temp[i];//還原剛剛備份的data
            thread[i]=ptemp[i];
        }

        forClientSockfd[num] = accept(sockfd,(struct sockaddr*)&clientInfo,&addrlen);
        int ptr = num;

        if(pthread_create( &(thread[num]), NULL, thread_function,(void*)&ptr)!=0) {
            printf("thread create failed");
        }
        threadid=thread[num];
        num++;
    }

    pthread_join(threadid,NULL);

    return 0;
}

void *thread_function(void *arg)
{
    char inputBuffer;//[256]={};
    int localindex = *((int*)arg);
    char ready='i' ;
    char message[512]="\nCommands:\na  add word to the tree\nf  find word in tree\ns  search words matching prefix\nd  delete word from the tree\nq  quit, freeing all data\n\nchoice: ";

    int size;
    while(1) {
        //printf("%d",sizeof(message));
        size = sizeof(message);
        //        send(forClientSockfd[localindex],&size,1,0);
        send(forClientSockfd[localindex],message,sizeof(message),0);//把上面的message傳給client
        recv(forClientSockfd[localindex],&inputBuffer,sizeof(inputBuffer),0);//receive client 要執行什麼選項
        printf("Get:thread %d :%c\n",localindex,inputBuffer);
        char sendinformation[256];
        if(inputBuffer=='a') {
            sprintf(sendinformation,"%s","serversend: a");
            send(forClientSockfd[localindex],sendinformation,sizeof(sendinformation),0);
        } else if(inputBuffer=='f') {
            sprintf(sendinformation,"%s","serversend: f");
            send(forClientSockfd[localindex],sendinformation,sizeof(sendinformation),0);
        } else if(inputBuffer=='s') {
            sprintf(sendinformation,"%s","serversend: s");
            send(forClientSockfd[localindex],sendinformation,sizeof(sendinformation),0);
        } else if(inputBuffer=='d') {
            sprintf(sendinformation,"%s","serversend: d");
            send(forClientSockfd[localindex],sendinformation,sizeof(sendinformation),0);
        } else if(inputBuffer=='q') {
            break;//跳出去準備關這個client的socket
        }
        recv(forClientSockfd[localindex],&ready,sizeof(ready),0);//試的時候連續傳送都會錯誤,所以這裡收一個dummy的東西,表示已經準備好跑下一次while傳訊息出去。
    }
    close(forClientSockfd[localindex]);//到這裡表示client要關閉了，就斷開與他的連結
}
