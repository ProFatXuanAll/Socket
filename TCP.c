#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "argsetup.h"
#include "TCP.h"
#include "ACK.h"

static unsigned long long int getFileSize(FILE* fptr);

extern void TCPS(char filename[], int cfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE], fbuffer[BUF_SIZE];
	unsigned long long int bytes_send, bytes_send_total, bytes_left;
	unsigned long long int ACK_counter;
	int flag;

	fptr = fopen(filename,"rb");

	if(fptr == NULL){	/* fopen function error check */
		perror("[error] file open failure: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}

	ACK_counter = 0;	/* IMPORTANT!!! */
	flag = 1;			/* IMPORTANT!!! */

	/* send file name */
	bytes_left = strlen(filename);
	bytes_send_total = 0;
	do{
		if(bytes_left >= BUF_SIZE - ACK_LEN){
			ACK_Set(&ACK_counter, buffer, filename + bytes_send_total, BUF_SIZE - ACK_LEN);
		}
		else{
			ACK_Set(&ACK_counter, buffer, filename + bytes_send_total, bytes_left);
			flag = 0;
			/* may need padding bits */
		}
		
		bytes_send = send(cfd, buffer, BUF_SIZE, 0);
		
		if(bytes_send < 0){
			perror("[error] on sendind filename: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}

		bytes_left -= (bytes_send - ACK_LEN);
		bytes_send_total += (bytes_send - ACK_LEN);
	}while(bytes_left > 0 && flag != 0);
	/* end send file name */

	/* special case when equal size, ACK for sending filename */
	if(flag){
		ACK_Set(&ACK_counter, buffer, "", 0);
		bytes_send = send(cfd, buffer, BUF_SIZE, 0);
	
		if(bytes_send < 0){
			perror("[error] on sendind filename: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}
	}
	/* end ACK for sending filename */

#ifdef DEBUG

	flag=1;	/* IMPORTANT!!! */

	/* send file content */
	while(!feof(fptr)){
		bytes_left = fread(fbuffer, sizeof(char), BUF_SIZE - ACK_LEN, fptr);
		bytes_send_total = 0;
		do{
			if(bytes_left >= BUF_SIZE - ACK_LEN){
				ACK_Set(&ACK_counter, buffer, fbuffer, BUF_SIZE - ACK_LEN);
			}
			else{
				ACK_Set(&ACK_counter, buffer, fbuffer, bytes_left);
				flag = 0;
				/* may need padding bits */
			}

			bytes_send = send(cfd, buffer, BUF_SIZE, 0);
			
			if(bytes_send < 0){
				perror("[error] on sendind file content: ");
				#ifdef LINUX
				close(cfd); 
				#endif
				exit(EXIT_FAILURE);
			}

			bytes_left -= (bytes_send - ACK_LEN);
			bytes_send_total += (bytes_send - ACK_LEN);
		}while(bytes_left > 0 && flag != 0);
	}
	
	/* special case when equal size, ACK for sending file content */
	if(flag){
		ACK_Set(&ACK_counter, buffer, "", 0);
		bytes_send = send(cfd, buffer, BUF_SIZE, 0);
	
		if(bytes_send < 0){
			perror("[error] on sendind file content: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}
	}
	/* end ACK for sending file content */

#endif
	fclose(fptr);
	/* end send file connent */
}

static void TCPR(int sfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE];
	char filename[NAME_SIZE_MAX];
	unsigned long long int bytes_recv, bytes_recv_total, bytes_write;
	unsigned long long int ACK_counter;
	int flag;

	/* receive file name first */
	bytes_recv_total = 0;
	while(1){
		if((recv(sfd, buffer, sizeof(buffer), 0)) < 0){
			perror("[error] on receiving filename: ");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		if(ACK_Check(&ACK_counter, buffer, &bytes_recv) < 0){
			perror("[error] on filename ACK check failed.");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		if(bytes_recv < BUF_SIZE - ACK_LEN){	/* ACK inform filename transfer end */
			strncpy(filename + bytes_recv_total, buffer, bytes_recv);
			bytes_recv_total += bytes_recv;
			filename[bytes_recv_total] = '\0';
			break;
		}
		strncpy(filename + bytes_recv_total, buffer, bytes_recv);
		bytes_recv_total += bytes_recv;
	}
	/* end receive file name */
	
	printf("[client] filename received: %s\n", filename);

	fptr = fopen(filename, "wb");
	
	if(fptr == NULL){	/* fopen function error check */
		perror("[error] file open failure: ");
		#ifdef LINUX
		close(sfd);
		#endif
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	/* receive file content */
	flag = 1;
	bytes_recv_total = 0;
	while(flag){
		if(bytes_recv = recv(sfd, buffer, sizeof(buffer), 0) < 0){
			perror("[error] on writing file: ");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		if(ACK_Check(&ACK_counter, buffer, &bytes_recv) < 0){
			perror("[error] on file ACK check failed.");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		if(bytes_recv < BUF_SIZE - ACK_LEN){	/* ACK inform file transfer end */
			if(bytes_recv == 0){
				break;	/* ACK end of transfer indicator */
			}
			else{
				flag = 0;
			}
		}
		bytes_write = fwrite(buffer, sizeof(char), bytes_recv, fptr);
		if(bytes_write < 0){
			perror("[error] on writing file: ");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		bytes_recv_total += bytes_recv;
	}
	/* end receive file content */

	printf("[client] file content received.\n");
	
#endif
	fclose(fptr);
	/* end receive file connent */
}

static unsigned long long int getFileSize(FILE* fptr)
{
	unsigned long long int fsize = 0;
	fseek(fptr, 0L, SEEK_END);
	fsize = ftell(fptr);
	rewind(fptr);
	return fsize;
}
