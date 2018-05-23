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
//---prefixsearch
#include <time.h>
#include <math.h>
#include "tst.h"
//#include "bench.c"
//---bloom filter
#include "bloom.h"
#define TableSize 5000000	//bloom filter 的大小
#define HashNumber 2	//有多少個 hash function
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL

#define BENCH_TEST_FILE "bench_ref.txt"

long poolsize = 2000000*WRDMAX;

/* simple trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}
static double tvgetf(void)
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}
#define IN_FILE "cities.txt"


int sockfd = 0;
int num =0;//現在有幾個client
int *forClientSockfd;//動態配置,存放每個client的descriper
pthread_t *thread;

void *thread_function(void *);
//-----memorypool還有bloom filter資訊是所有thread共享,放global
char * pool;
char * Top;
tst_node *root = NULL;
bloom_t bloom;
int idx=0;

int main(int argc, char **argv)
{
//-------------------------------------------------------------插入資料進tire
//   	char word[WRDMAX] = "";
//   	char *sgl[LMAX] = {NULL};
    int rtn = 0;
    //	int  sidx = 0;
//   	tst_node * res = NULL;
    FILE *fp = fopen(IN_FILE, "r");
    double t1, t2;

    if (!fp) { /* prompt, open, validate file for reading */
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }
    t1 = tvgetf();

//先 create出一個 bloom filter
    bloom = bloom_create(TableSize);

//******memorypool*****
    pool = (char *) malloc(poolsize * sizeof(char));
    Top = pool;
    while ((rtn = fscanf(fp, "%s",Top)) != EOF) {
        char *p = Top;
        /* insert reference to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {//沒有insert成功

            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fp);
            return 1;
        } else { //有insert 進資料結構，因此也要加入bloom filter
            bloom_add(bloom,Top);
        }
        idx++;
        Top += (strlen(Top) + 1);
    }
    t2 = tvgetf();
    fclose(fp);
    printf("ternary_tree, loaded %d words in %.6f sec\n", idx,t2-t1);

//--------------------------------------------------------------------建立socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線
    //提供socket訊息
    struct sockaddr_in serverInfo,clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
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
    char *inputBuffer=malloc(sizeof(char)*1024);//[256]={};
    int localindex = *((int*)arg);
    char ready='i' ;
    //char * message = malloc(sizeof(char)*256);
    //sprintf(message,"%s","\nCommands:\na  add word to the tree\nf  find word     in tree\ns  search words matching prefix\nd  delete word from the tree\nq      quit, freeing all data\n\nchoice: ");
    char message[1024];
//    int size;

    //------------------prefix srarch--------
    double t1, t2;
    char *p;
    tst_node * res = NULL;
//	char *sgl[LMAX] = {NULL};
    //int rtn=0,sidx=0;
    while(1) {
        sprintf(message,"%s","\nCommands:\na  add word to the tree\nf  find word in tree\ns  search words matching prefix\nd  delete word from the tree\nq  quit, freeing all data\n\nchoice: ");
        send(forClientSockfd[localindex],message,sizeof(message),0);//把上面的message傳給client
        recv(forClientSockfd[localindex],inputBuffer,sizeof(inputBuffer),0);//receive client 要執行什麼選項
        printf("Get from thread %d :%s\n",localindex,inputBuffer);
        //char * sendinformation=malloc(sizeof(char)*256);

        if(strcmp(inputBuffer,"a")==0) {//選項a: add word
            sprintf( message,"%s","enter word to add: ");
            send(forClientSockfd[localindex],message,sizeof(message),0);//傳enter word to add:給client
            recv(forClientSockfd[localindex],inputBuffer,sizeof(inputBuffer),0);//拿到要加入的word
            printf("Get from thread %d :%s\n",localindex,inputBuffer);

            sprintf(Top,"%s",inputBuffer);//把接收到的字串給Top
            rmcrlf(Top);

            p = Top;
            t1 = tvgetf();
            /*insert reference to each string */
            if(bloom_test(bloom,Top)==1)//已經被filter偵測存在，不要走tree
                res=NULL;
            else { //否則就去走訪tree加入,並加入bloom filter
                bloom_add(bloom,Top);
                res = tst_ins_del(&root, &p, INS, REF);
            }
            t2 = tvgetf();
            if (res) {//如果res!=NULL表示有 insert 成功
                idx++;
                Top += (strlen(Top) + 1);
                printf("  %s - inserted in %.10f sec. (%d words in tree)\n",
                       (char *) res, t2 - t1,idx);
                //把成功的訊息傳給client
                sprintf(message,"  %s - inserted in %.10f sec. (%d words in tree)\n",(char *) res, t2 - t1,idx);
                send(forClientSockfd[localindex],message,sizeof(message),0);
            } else {//否則失敗

                printf("  %s - already exists in list.\n", (char *) res);
                //把失敗的訊息傳給client
                sprintf(message,"  %s - already exists in list.\n", (char *) res);
                send(forClientSockfd[localindex],message,sizeof(message),0);
            }
        } else if(strcmp(inputBuffer,"f")==0) {
            sprintf(message,"%s","serversend: f");
            send(forClientSockfd[localindex],message,sizeof(message),0);
        } else if(strcmp(inputBuffer,"s")==0) {
            sprintf(message,"%s","serversend: s");
            send(forClientSockfd[localindex],message,sizeof(message),0);
        } else if(strcmp(inputBuffer,"d")==0) {
            sprintf(message,"%s","serversend: d");
            send(forClientSockfd[localindex],message,sizeof(message),0);
        } else if(strcmp(inputBuffer,"q")==0) {
            break;//跳出去準備關這個client的socket
        }
        recv(forClientSockfd[localindex],&ready,sizeof(ready),0);//試的時候連續傳送都會錯誤,所以這裡收一個dummy的東西,表示已經準備好跑下一次while傳訊息出去。
    }
    close(forClientSockfd[localindex]);//到這裡表示client要關閉了，就斷開與他的連結
    return 0;
}
