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

void *ServerEcho(void *args)
{
    int clientFileDescriptor=(intptr_t)args;
    char str[COM_BUFF_SIZE]; // array to hold client string
    read(clientFileDescriptor,str, COM_BUFF_SIZE); // read client string into str array
    printf("reading from client:%s\n",str);

    ClientRequest rqst;

    // Tokenize string data and set it to object attributes
    ParseMsg(str, &rqst); 
    
    // CRITICAL SECTION

    //GET_TIME(start_time);
    if (rqst.is_read == 0){
        
        // Write case -> server will update corresponding string in array with new text supplied by client.
        pthread_rwlock_wrlock(&readwritelock); // Write lock -> no other threads may write to the array
        GET_TIME(timeStart);
        // Server will then send the updated string from array to the client
        setContent(str, rqst.pos, theArray);

        pthread_rwlock_unlock(&readwritelock);

        pthread_rwlock_rdlock(&readwritelock); // Read lock -> no other threads here may read from the array

        getContent(str, rqst.pos, theArray);

        GET_TIME(timeEnd);
        timeList[request] = timeEnd - timeStart;
        request++;
        pthread_rwlock_unlock(&readwritelock);
    } else if (rqst.is_read == 1){

        // Read case -> server will just send back the corresponding string to the client
        pthread_rwlock_rdlock(&readwritelock); // Read lock -> no other threads here may read from the array
        GET_TIME(timeStart);
        getContent(str, rqst.pos, theArray);
        GET_TIME(timeEnd);
        timeList[request] = timeEnd - timeStart;
        request++;
        pthread_rwlock_unlock(&readwritelock);
    }

    // END CRITICAL SECTION

    write(clientFileDescriptor,str,COM_BUFF_SIZE);
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