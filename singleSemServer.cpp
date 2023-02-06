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
sem_t semaphore;
double start_time;
double end_time;
double elapsed;
double* times;

void *ServerEcho(void *args)
{
    int clientFileDescriptor=(intptr_t)args;
    char str[COM_BUFF_SIZE]; // array to hold client string
    char dst[COM_BUFF_SIZE]; // destination array to put theArray string into
    read(clientFileDescriptor,str, COM_BUFF_SIZE); // read client string into str array
    printf("reading from client:%s\n",str);

    ClientRequest rqst;

    // Tokenize string data and set it to object attributes
    ParseMsg(str, &rqst); 
    
    // CRITICAL SECTION
    sem_wait(&semaphore);
    GET_TIME(start_time);
    if (rqst.is_read == 0){
        // Write case -> server will update corresponding string in array with new text supplied by client.
        // Server will then send the updated string from array to the client
        setContent(str, rqst.pos, theArray);
    }
    // Read case -> server will just send back the corresponding string to the client
    getContent(dst, rqst.pos, theArray);
    GET_TIME(end_time);
    sem_post(&semaphore);
    // END CRITICAL SECTION

    write(clientFileDescriptor,str,COM_BUFF_SIZE);
    elapsed = end_time - start_time;
    saveTimes(times, elapsed);
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
    pthread_t t[20];

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

    // Initialize our semaphore
    sem_init(&semaphore, 0, 1);

    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");
        listen(serverFileDescriptor, COM_NUM_REQUEST); //changed '2000' to COM_NUM_REQUEST

        while(1)        //loop infinity, wait for client connections
        {
            for(i=0;i<20;i++)      //can support 20 clients at a time
            {
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                printf("Connected to client %d\n",clientFileDescriptor);
                pthread_create(&t[i],NULL,ServerEcho,(void *)(long)clientFileDescriptor);
            }
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
    return 0;
}