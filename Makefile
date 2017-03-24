CC=gcc

all: lab1_file_transfer
	
lab1_file_transfer: main.o server.o client.o TCP.o UDP.o ACK.o argsetup.h
	${CC} -o lab1_file_transfer main.o server.o client.o ACK.o TCP.o UDP.o
	cp lab1_file_transfer ../
	cp lab1_file_transfer ../testsock
main.o: main.c server.h client.c argsetup.h
	gcc -c main.c
server.o: server.c server.h TCP.h UDP.h argsetup.h
	gcc -c server.c
client.o: client.c client.h TCP.h UDP.h argsetup.h
	gcc -c client.c
TCP.o: TCP.c TCP.h ACK.h argsetup.h
	gcc -c TCP.c
UDP.o: UDP.c UDP.h ACK.h argsetup.h
	gcc -c UDP.c
ACK.o: ACK.c ACK.h argsetup.h
	gcc -c ACK.c

clean:
	rm lab1_file_transfer *.o
