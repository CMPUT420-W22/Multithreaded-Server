#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<stdint.h>
#include<semaphore.h>
#include<iostream>
using namespace std;
extern "C" {
    #include "common.h"
    #include "timer.h"
}

// Initialize global variables

char **theArray;
double timeStart;
double timeEnd;
double timeList[COM_NUM_REQUEST] = {0};
double* timeptr = &timeList[0];  
int request = 0;

pthread_rwlock_t *readwritelock;
pthread_mutex_t indexlock = PTHREAD_MUTEX_INITIALIZER;

void *ServerEcho(void *args)
{
    int clientFileDescriptor=(intptr_t)args;
    char str[COM_BUFF_SIZE]; // array to hold client string
    char dst[COM_BUFF_SIZE]; // array to hold string to send to client
    read(clientFileDescriptor,str, COM_BUFF_SIZE); // read client string into str array
    printf("reading from client:%s\n",str);

    ClientRequest rqst;

    // Tokenize string data and set it to object attributes
    ParseMsg(str, &rqst); 
    
    if (rqst.is_read == 0){
        // Write case -> server will update corresponding string in array with new text supplied by client.
        GET_TIME(timeStart);

        // Write lock -> no other threads may write to the array element
        pthread_rwlock_wrlock(&readwritelock[rqst.pos]); 
  
        // Set the element in the array to the string
        setContent(rqst.msg, rqst.pos, theArray);
        // Get the message to send back
        getContent(str, rqst.pos, theArray);
        // Remove the write lock from this array element
        pthread_rwlock_unlock(&readwritelock[rqst.pos]);

        GET_TIME(timeEnd);

        // Write the string to the client fd
        write(clientFileDescriptor,str,COM_BUFF_SIZE);

    } else {
        // Read case -> server will just send back the corresponding string to the client
        // Blocks if a thread holds the lock for writing and no threads are waiting on the lock
        GET_TIME(timeStart);

        // Read lock -> no other threads here may read from this array element
        pthread_rwlock_rdlock(&readwritelock[rqst.pos]);  
        // save string in position 'pos' from theArray to dst[]
        getContent(dst, rqst.pos, theArray); 
        // Remove the readlock from this array element
        pthread_rwlock_unlock(&readwritelock[rqst.pos]);

        GET_TIME(timeEnd);

        // Write the string to the client fd
        write(clientFileDescriptor,dst,COM_BUFF_SIZE);

    }

    // Lock the index so we can save the time to the correct array index
    pthread_mutex_lock(&indexlock);
    timeList[request] = timeEnd - timeStart;
    request++;
    pthread_mutex_unlock(&indexlock);

    close(clientFileDescriptor);

    return NULL;
}

int main(int argc, char* argv[])
{
    struct sockaddr_in sock_var;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
    int clientFileDescriptor;
    int i;

    // Array of threads to hold COM_NUM_REQUEST threads
    pthread_t t[COM_NUM_REQUEST];

    int elements = atoi(argv[1]);

    // Allocate space for the array of strings
    theArray = (char**)malloc (elements * sizeof(theArray[0]));
    // Allocate space for the array of rwlocks
    readwritelock = (pthread_rwlock_t*)malloc(elements * sizeof(pthread_rwlock_t));

    //allocating "n" amount of space for theArray
    //https://stackoverflow.com/questions/4316987/define-the-size-of-a-global-array-from-the-command-line
    for (int i = 0; i < elements; i++){
        // Allocate space for the individual elements
        theArray[i] = (char*)malloc(COM_BUFF_SIZE * sizeof(char));
        pthread_rwlock_init(&readwritelock[i],NULL);
    }

    // changed socket commands to implement command line arguments
    sock_var.sin_addr.s_addr=inet_addr(argv[2]);
    sock_var.sin_port= atoi(argv[3]);
    sock_var.sin_family=AF_INET;


    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");
        listen(serverFileDescriptor, COM_NUM_REQUEST); 

        while(1)       
        {
            for(i=0;i<COM_NUM_REQUEST;i++) // Loop for COM_NUM_REQUEST threads
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                printf("Connected to client %d\n",clientFileDescriptor);
                pthread_create(&t[i],NULL,ServerEcho,(void *)(long)clientFileDescriptor);
            }
            for (i=0; i<COM_NUM_REQUEST; i++){
                pthread_join(t[i], NULL);
            }
            // Save the average memory access latency to process 1000 requests
            saveTimes(timeptr, COM_NUM_REQUEST);

        }
        close(serverFileDescriptor);
    }
    else{
        printf("socket creation failed\n");
    }
    // Free the allocated space
    for (int i = 0; i < elements; i++){
        free(theArray[i]);
        pthread_rwlock_destroy(&readwritelock[i]);
    }
    free(theArray);
    free(readwritelock);
    return 0;
}