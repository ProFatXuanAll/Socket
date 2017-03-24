CC=gcc

all: lab1_file_transfer
	
lab1_file_transfer: lab1_file_transfer.c argsetup.h
	${CC} -o lab1_file_transfer lab1_file_transfer.c
	cp lab1_file_transfer ../
	cp lab1_file_transfer ../testsock
clean:
	rm lab1_file_transfer
