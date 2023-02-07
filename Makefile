all: 

singleMutexServer: singleMutexServer.o
	g++ -o singleMutexServer singleMutexServer.o -pthread
	
singleMutexServer.o: singleMutexServer.cpp timer.h common.h
	g++ -c -o singleMutexServer.o singleMutexServer.cpp

singleReadServer: singleReadServer.o
	g++ -o singleReadServer singleReadServer.o -pthread
	
singleReadServer.o: singleReadServer.cpp timer.h common.h
	g++ -c -o singleReadServer.o singleReadServer.cpp

multReadServer: multReadServer.o
	g++ -o multReadServer multReadServer.o -pthread
	
multReadServer.o: multReadServer.cpp timer.h common.h
	g++ -c -o multReadServer.o multReadServer.cpp	

multMutexServer: multMutexServer.o
	g++ -o multMutexServer multMutexServer.o -pthread
	
multMutexServer.o: multMutexServer.cpp timer.h common.h
	g++ -c -o multMutexServer.o multMutexServer.cpp	

client: client.o
	gcc -o client client.o -pthread
	
client.o: client.c common.h
	gcc -c -o client.o client.c

attacker: attacker.o
	gcc -o attacker attacker.o -pthread -lm
	
attacker.o: attacker.c common.h
	gcc -c -o attacker.o attacker.c 
	
clean: 
	rm *.o
	rm singleMutexServer
	rm multMutexServer
	rm singleReadServer
	rm multReadServer
	rm attacker
	rm client
	rm server_output_time_aggregated
