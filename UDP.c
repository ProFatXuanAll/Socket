#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "argsetup.h"
#include "UDP.h"
#include "ACK.h"
#include "log.h"

static unsigned long long int getFileSize(FILE* fptr);

extern void UDPS(char filename[], int cfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE], fbuffer[BUF_SIZE];
	unsigned long long int bytes_send, bytes_send_total, bytes_left, bytes_len_total;
	unsigned long long int ACK_counter;

	fptr = fopen(filename,"rb");

	if(fptr == NULL){	/* fopen function error check */
		perror("[error] file open failure: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}

	ACK_counter = 0;	/* IMPORTANT!!! */

	/* send file name */
	bytes_left = strlen(filename);
	bytes_send_total = 0;
	do{
		memset(buffer, 0, BUF_SIZE);
		if(bytes_left >= (BUF_SIZE - ACK_LEN)){	/* enough to fill in (BUF_SIZE - ACK_LEN) bytes */
			ACK_Set(&ACK_counter, buffer, filename + bytes_send_total, BUF_SIZE - ACK_LEN);
			bytes_send = send(cfd, buffer, BUF_SIZE, 0);
			bytes_left -= (bytes_send - ACK_LEN);
			bytes_send_total += (bytes_send - ACK_LEN);
		}
		else if(bytes_left > 0){	/* not enough to fill in (BUF_SIZE - ACK_LEN) bytes */
			ACK_Set(&ACK_counter, buffer, filename + bytes_send_total, bytes_left);
			bytes_send = send(cfd, buffer, BUF_SIZE, 0);
			if(bytes_send == BUF_SIZE){	/* all send */
				bytes_left = 0;
				bytes_send_total += (bytes_left);
			}
			else{	/* some bytes left */
				bytes_left -= (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);
				bytes_send_total += (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);
			}
			/* may need padding bits */
		}
		else{	/* error */
			fprintf(stderr, "[error] unexpected bytes left on sending file name.\n");
			exit(EXIT_FAILURE);
		}
		
		if(bytes_send < 0){
			perror("[error] on sendind filename: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}
	}while(bytes_left != 0);
	/* end send file name */
	
	/* ACK for sending filename */
	ACK_Set(&ACK_counter, buffer, "", 0);
	bytes_send = send(cfd, buffer, BUF_SIZE, 0);
	
	if(bytes_send < 0){
		perror("[error] on sendind filename: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}
	/* end ACK for sending filename */

	/* send file length */
	sprintf(fbuffer, "%llu", getFileSize(fptr));
	bytes_left = strlen(fbuffer);
	bytes_send_total = 0;
	do{
		memset(buffer, 0, BUF_SIZE);
		if(bytes_left >= (BUF_SIZE - ACK_LEN)){	/* enough to fill in (BUF_SIZE - ACK_LEN) bytes */
			ACK_Set(&ACK_counter, buffer, fbuffer + bytes_send_total, BUF_SIZE - ACK_LEN);
			bytes_send = send(cfd, buffer, BUF_SIZE, 0);
			bytes_left -= (bytes_send - ACK_LEN);
			bytes_send_total += (bytes_send - ACK_LEN);
		}
		else if(bytes_left > 0){	/* not enough to fill in (BUF_SIZE - ACK_LEN) bytes */
			ACK_Set(&ACK_counter, buffer, fbuffer + bytes_send_total, bytes_left);
			bytes_send = send(cfd, buffer, BUF_SIZE, 0);
			if(bytes_send == BUF_SIZE){	/* all send */
				bytes_left = 0;
				bytes_send_total += (bytes_left);
			}
			else{	/* some bytes left */
				bytes_left -= (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);	
				bytes_send_total += (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);
			}
			/* may need padding bits */
		}
		else{	/* error */
			fprintf(stderr, "[error] unexpected bytes left on sending file length.\n");
			exit(EXIT_FAILURE);
		}
		
		if(bytes_send < 0){
			perror("[error] on sendind file length: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}
	}while(bytes_left != 0);
	/* end send file length */
	
	/* ACK for sending file length */
	ACK_Set(&ACK_counter, buffer, "", 0);
	bytes_send = send(cfd, buffer, BUF_SIZE, 0);
	
	if(bytes_send < 0){
		perror("[error] on sendind filename: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}
	/* end ACK for sending file length */

	/* send file content */
	bytes_len_total = getFileSize(fptr);
	while(bytes_len_total != 0){
		memset(fbuffer, 0, BUF_SIZE);
		bytes_left = fread(fbuffer, sizeof(char), BUF_SIZE - ACK_LEN, fptr);
		bytes_send_total = 0;
		do{
			memset(buffer, 0, BUF_SIZE);
			if(bytes_left == (BUF_SIZE - ACK_LEN)){
				ACK_Set(&ACK_counter, buffer, fbuffer, BUF_SIZE - ACK_LEN);
				bytes_send = send(cfd, buffer, BUF_SIZE, 0);
				bytes_left -= (bytes_send - ACK_LEN);
				bytes_send_total += (bytes_send - ACK_LEN);
			}
			else if(bytes_left < BUF_SIZE - ACK_LEN){
				ACK_Set(&ACK_counter, buffer, fbuffer, bytes_left);
				bytes_send = send(cfd, buffer, BUF_SIZE, 0);
				if(bytes_send == BUF_SIZE){	/* all send */
					bytes_left = 0;
					bytes_send_total += (bytes_left);
				}
				else{	/* some bytes left */
					bytes_left -= (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);
					bytes_send_total += (bytes_send > ACK_LEN ? bytes_send - ACK_LEN : 0);
				}
				/* may need padding bits */
			}
			else{
				fprintf(stderr, "[error] unexpected block size.\n");
				exit(EXIT_FAILURE);
			}
			
			if(bytes_send < 0){
				perror("[error] on sendind file content: ");
				#ifdef LINUX
				close(cfd); 
				#endif
				exit(EXIT_FAILURE);
			}

		}while(bytes_left != 0);

		bytes_len_total -= bytes_send_total;
	}
	
	/* ACK for sending file content */
	ACK_Set(&ACK_counter, buffer, "", 0);
	bytes_send = send(cfd, buffer, BUF_SIZE, 0);
	
	if(bytes_send < 0){
		perror("[error] on sendind file content: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}
	/* end ACK for sending file content */

	fclose(fptr);
	/* end send file connent */
}

extern void UDPR(int sfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE];
	char filename[NAME_SIZE_MAX];
	char filelength[BUF_SIZE];
	unsigned long long int bytes_recv, bytes_recv_total, bytes_write, bytes_len_total;
	unsigned long long int ACK_counter;

	/* receive file name first */
	bytes_recv_total = 0;
	while(1){
		memset(buffer, 0, BUF_SIZE);
		if((bytes_recv=recv(sfd, buffer, sizeof(buffer), 0)) < 0){
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
		if(bytes_recv == 0){	/* ACK inform filename transfer end */
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

	/* receive file length */
	bytes_recv_total = 0;
	while(1){
		memset(buffer, 0, BUF_SIZE);
		if((bytes_recv=recv(sfd, buffer, sizeof(buffer), 0)) < 0){
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
		if(bytes_recv == 0){	/* ACK inform filename transfer end */
			filelength[bytes_recv_total] = '\0';
			break;
		}
		strncpy(filelength + bytes_recv_total, buffer, bytes_recv);
		bytes_recv_total += bytes_recv;
	}
	/* end receive file length */
	
	bytes_len_total = strtoull(filelength,NULL,10);

	printf("[client] file length received: %llu\n", bytes_len_total);

	/* receive file content */
	bytes_recv_total = 0;
	while(1){
		memset(buffer, 0, BUF_SIZE);
		if((bytes_recv = recv(sfd, buffer, sizeof(buffer), 0)) < 0){
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
		if(bytes_recv == 0){	/* ACK inform file transfer end */
			break;
		}
		bytes_write = fwrite(buffer, sizeof(char), bytes_recv, fptr);
		if(bytes_write < 0 || bytes_write != bytes_recv){
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
