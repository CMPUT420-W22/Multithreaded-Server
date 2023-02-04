all: 

singleMutexServer: singleMutexServer.o
	g++ -o singleMutexServer singleMutexServer.o -pthread
	
singleMutex.o: singleMutexServer.cpp timer.h common.h
	g++ -c -o singleMutexServer.o singleMutexServer.cpp

client: client.o
	gcc -o client client.o -pthread
	
client.o: client.c common.h
	gcc -c -o client.o client.c

test: 
	
clean: 
	rm *.o
