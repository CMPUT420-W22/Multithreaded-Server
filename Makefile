all: main1 main2 main3 main4 client attacker

main1: singleMutexServer.o
	g++ -o main1 singleMutexServer.o -pthread
	
singleMutexServer.o: singleMutexServer.cpp timer.h common.h
	g++ -c -o singleMutexServer.o singleMutexServer.cpp

main2: singleReadServer.o
	g++ -o main2 singleReadServer.o -pthread
	
singleReadServer.o: singleReadServer.cpp timer.h common.h
	g++ -c -o singleReadServer.o singleReadServer.cpp

main3: multReadServer.o
	g++ -o main3 multReadServer.o -pthread
	
multReadServer.o: multReadServer.cpp timer.h common.h
	g++ -c -o multReadServer.o multReadServer.cpp	

main4: multMutexServer.o
	g++ -o main4 multMutexServer.o -pthread
	
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
	rm main1
	rm main2
	rm main3
	rm main4
	rm attacker
	rm client
	rm server_output_time_aggregated
