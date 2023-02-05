#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include <iostream>
#include "common.h"
#include "timer.h"
using namespace std;
//using simpleServer.c from provided in /demos  
//global variables
char** theArray; 
pthread_mutex_t mutex;
//to implement the saveTimes 
double timeStart;
double timeEnd;
double timeList[COM_NUM_REQUEST] = {0};
double* timeptr = &timeList[0];  
int request = 0; 
void *ServerEcho(void *args)
{
    int clientFileDescriptor=(intptr_t)args;
    char str[COM_BUFF_SIZE];
    read(clientFileDescriptor,str,COM_BUFF_SIZE);

    printf("reading from client:%s\n",str);
    //using methods in common.h to read in client request
    ClientRequest rqst; 
    ParseMsg(str,&rqst);
    
    //**critical section**//
    pthread_mutex_lock(&mutex);
    //after the server finishs receivin the client request
    //putting time params in mutexlock so it is modified once at a time.
    GET_TIME(timeStart);
    
    if (rqst.is_read == 0){
        //write to array 
        setContent(rqst.msg,rqst.pos,theArray);
    }
    //getting the string to send back to the client stored in str
    getContent(str, rqst.pos, theArray);
    //before the server sends the msg to the client
    GET_TIME(timeEnd);
    //adding latency to time
    timeList[request] = timeEnd - timeStart;
    request++;
    pthread_mutex_unlock(&mutex);
    //**Critical Section**//
  
    write(clientFileDescriptor,str,COM_BUFF_SIZE);
    memset(str, 0, sizeof(str));
    close(clientFileDescriptor);
    return NULL;
}


int main(int argc, char* argv[])
{
    //cmd line args ./executable {arraySize} {serverIP} {ServerPort}
    struct sockaddr_in sock_var;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
    int clientFileDescriptor;
    int i;
    pthread_t t[COM_NUM_REQUEST];
    pthread_mutex_init(&mutex, NULL);
    
    //allocating "n" amount of space for theArray
    //https://stackoverflow.com/questions/4316987/define-the-size-of-a-global-array-from-the-command-line
    int elements = atoi(argv[1]);
    theArray = (char**)malloc (elements * sizeof (theArray[0]));
    for (int i = 0; i < elements; i++){
        theArray[i] = (char*)malloc(COM_BUFF_SIZE*sizeof(char));
    }

    sock_var.sin_addr.s_addr=inet_addr(argv[2]);
    sock_var.sin_port=atoi(argv[3]);
    
    sock_var.sin_family=AF_INET;
    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");

        listen(serverFileDescriptor,COM_NUM_REQUEST); 
        while(1)        //loop infinity
        {
            for(i=0;i<COM_NUM_REQUEST;i++)      //can support 20 clients at a time
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                printf("Connected to client %d\n",clientFileDescriptor);
                pthread_create(&t[i],NULL,ServerEcho,(void *)(long)clientFileDescriptor);
            }
            for(i=0;i<COM_NUM_REQUEST;i++)      //can support 20 clients at a time
            {
                pthread_join(t[i], NULL);
            } 
        }

        close(serverFileDescriptor);
    }
    else{
        printf("socket creation failed\n");
    }
    //free allocated space
    saveTimes(timeptr, COM_NUM_REQUEST);
    for (int i = 0; i < elements; i++){
        free(theArray[i]); 
    }
    free(theArray);
    pthread_mutex_destroy(&mutex);

    return 0;
}