CC=gcc

all: lab1_file_transfer
	
lab1_file_transfer: main_linux.o server.o client.o TCP.o UDP.o ACK.o log.o argsetup.h
	${CC} -o lab1_file_transfer main_linux.o server.o client.o TCP.o UDP.o ACK.o log.o
	cp lab1_file_transfer ../
	cp lab1_file_transfer ../testsock
main_linux.o: main_linux.c server.h client.c argsetup.h ACK.h
	gcc -c main_linux.c
server.o: server.c server.h TCP.h UDP.h argsetup.h
	gcc -c server.c
client.o: client.c client.h TCP.h UDP.h argsetup.h
	gcc -c client.c
TCP.o: TCP.c TCP.h log.h argsetup.h
	gcc -c TCP.c
UDP.o: UDP.c UDP.h ACK.h log.h argsetup.h
	gcc -c UDP.c
ACK.o: ACK.c ACK.h argsetup.h
	gcc -c ACK.c
log.o: log.c log.h
	gcc -c log.c

clean:
	rm lab1_file_transfer *.o
