#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<cstdint>
#include<semaphore.h>
#include<iostream>
using namespace std;
extern "C" {
    #include "common.h"
    #include "timer.h"
}

char **theArray;

double timeStart;
double timeEnd;
double timeList[COM_NUM_REQUEST] = {0};
double* timeptr = &timeList[0];  
int request = 0;

pthread_rwlock_t readwritelock;
pthread_mutex_t indexlock = PTHREAD_MUTEX_INITIALIZER;

void *ServerEcho(void *args)
{
    int clientFileDescriptor=(intptr_t)args;
    char str[COM_BUFF_SIZE]; // array to hold client string
    char dst[COM_BUFF_SIZE];
    read(clientFileDescriptor,str, COM_BUFF_SIZE); // read client string into str array
    printf("reading from client:%s\n",str);

    ClientRequest rqst;

    // Tokenize string data and set it to object attributes
    ParseMsg(str, &rqst); 
    
    // CRITICAL SECTION

    if (rqst.is_read == 0){
        // Write case -> server will update corresponding string in array with new text supplied by client.

        GET_TIME(timeStart);

        pthread_rwlock_wrlock(&readwritelock); // Write lock -> no other threads may write to the array
  
        // Server will then send the updated string from array to the client
        setContent(rqst.msg, rqst.pos, theArray);
        getContent(str, rqst.pos, theArray);
        pthread_rwlock_unlock(&readwritelock);
        GET_TIME(timeEnd);
        if (write(clientFileDescriptor,str,COM_BUFF_SIZE) < 0){
            cout << "server write error" << endl;
        }

    } else {
        GET_TIME(timeStart);
        // Read case -> server will just send back the corresponding string to the client
        // Blocks if a thread holds the lock for writing and no threads are waiting on the lock
        pthread_rwlock_rdlock(&readwritelock);  // Read lock -> no other threads here may read from the array

        getContent(dst, rqst.pos, theArray); // save string in position 'pos' from theArray to dst
        pthread_rwlock_unlock(&readwritelock);

        GET_TIME(timeEnd);
        if (write(clientFileDescriptor,dst,COM_BUFF_SIZE) < 0){
            cout << "server read error" << endl;
        }

    }

    pthread_mutex_lock(&indexlock);
    timeList[request] = timeEnd - timeStart;
    request++;
    pthread_mutex_unlock(&indexlock);
    // END CRITICAL SECTION
    close(clientFileDescriptor);
    return NULL;
}

// size of string array, server ip, server port
int main(int argc, char* argv[])
{
    struct sockaddr_in sock_var;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
    int clientFileDescriptor;
    int i;
    pthread_t t[COM_NUM_REQUEST];
    pthread_rwlock_init(&readwritelock,NULL);

    //allocating "n" amount of space for theArray
    //https://stackoverflow.com/questions/4316987/define-the-size-of-a-global-array-from-the-command-line
    int elements = atoi(argv[1]);
    theArray = (char**)malloc (elements * sizeof(theArray[0]));
    for (int i = 0; i < elements; i++){
        theArray[i] = (char*)malloc(COM_BUFF_SIZE * sizeof(char));
    }

    // changed socket commands to implement command line arguments
    sock_var.sin_addr.s_addr=inet_addr(argv[2]);
    sock_var.sin_port= atoi(argv[3]);
    sock_var.sin_family=AF_INET;


    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");
        listen(serverFileDescriptor, COM_NUM_REQUEST); //changed '2000' to COM_NUM_REQUEST

        while(1)        //loop infinity, wait for client connections
        {
            for(i=0;i<COM_NUM_REQUEST;i++)      //change '20' to COM_NUM_REQUEST support 20 clients at a time
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                printf("Connected to client %d\n",clientFileDescriptor);
                pthread_create(&t[i],NULL,ServerEcho,(void *)(long)clientFileDescriptor);
            }
            for (i=0; i<COM_NUM_REQUEST; i++){
                pthread_join(t[i], NULL);
            }
            saveTimes(timeptr, COM_NUM_REQUEST);

        }
        close(serverFileDescriptor);
    }
    else{
        printf("socket creation failed\n");
    }
    for (int i = 0; i < elements; i++){
        free(theArray[i]);
    }
    free(theArray);
    pthread_rwlock_destroy(&readwritelock);
    return 0;
}