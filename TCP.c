#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "argsetup.h"
#include "TCP.h"
#include "log.h"

static unsigned long long int getFileSize(FILE* fptr);

extern void TCPS(char filename[], int cfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE], fbuffer[BUF_SIZE];
	unsigned long long int bytes_send, bytes_left, bytes_len_total, buf_ptr;

	fptr = fopen(filename,"rb");

	if(fptr == NULL){	/* fopen function error check */
		perror("[error] file open failure: ");
		#ifdef LINUX
		close(cfd); 
		#endif
		exit(EXIT_FAILURE);
	}

	/* send file name */
	memset(buffer, 0, BUF_SIZE);
	sprintf(buffer,"%s",filename);
	bytes_left = BUF_SIZE;
	buf_ptr = 0;
	do{
		bytes_send = send(cfd, buffer + buf_ptr, BUF_SIZE - buf_ptr, 0);
		
		if(bytes_send < 0){
			perror("[error] on sendind filename: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}

		bytes_left -= bytes_send;
		buf_ptr += bytes_send;
	}while(bytes_left!=0);
	/* end send file name */

	/* send file length */
	memset(buffer, 0, BUF_SIZE);
	sprintf(buffer, "%llu", getFileSize(fptr));
	bytes_left = BUF_SIZE;
	buf_ptr=0;
	do{
		bytes_send = send(cfd, buffer + buf_ptr, BUF_SIZE - buf_ptr, 0);
		
		if(bytes_send < 0){
			perror("[error] on sendind file length: ");
			#ifdef LINUX
			close(cfd); 
			#endif
			exit(EXIT_FAILURE);
		}

		bytes_left -= bytes_send;
		buf_ptr += bytes_send;
	}while(bytes_left!=0);
	/* end send file length */

	/* send file content */
	if(getFileSize(fptr)!=0){
		bytes_len_total = getFileSize(fptr);

		while(bytes_len_total != 0){
			memset(buffer, 0, BUF_SIZE);
			bytes_left = (bytes_len_total > BUF_SIZE ? BUF_SIZE : bytes_len_total);
			buf_ptr = 0;
			
			do{
				buf_ptr += fread(buffer + buf_ptr, sizeof(char), bytes_left - buf_ptr, fptr);
			}while(buf_ptr != bytes_left);
			
			buf_ptr = 0;
			
			do{
				bytes_send = send(cfd, buffer + buf_ptr, bytes_left - buf_ptr, 0);
			
				if(bytes_send < 0){
					perror("[error] on sendind file content: ");
					#ifdef LINUX
					close(cfd); 
					#endif
					exit(EXIT_FAILURE);
				}

				buf_ptr += bytes_send;
			}while(buf_ptr != bytes_left);

			bytes_len_total -= bytes_left;
		}
	}

	fclose(fptr);
	/* end send file connent */
}

extern void TCPR(int sfd)
{
	FILE* fptr;
	char buffer[BUF_SIZE];
	char filename[BUF_SIZE];
	char filelength[BUF_SIZE];
	unsigned long long int bytes_recv, bytes_len_total, bytes_write, buf_ptr;
	unsigned long long int numerator, denominator, next;

	/* receive file name first */
	buf_ptr = 0;
	memset(filename, 0, BUF_SIZE);
	do{
		bytes_recv = recv(sfd, filename + buf_ptr, BUF_SIZE - buf_ptr, 0);
		
		if(bytes_recv < 0){
			perror("[error] on receiving filename: ");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		
		buf_ptr += bytes_recv;
	}while(buf_ptr != BUF_SIZE);
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
	buf_ptr = 0;
	memset(filelength, 0, BUF_SIZE);
	do{
		bytes_recv = recv(sfd, filelength + buf_ptr, BUF_SIZE - buf_ptr, 0);
		
		if(bytes_recv < 0){
			perror("[error] on receiving filename: ");
			#ifdef LINUX
			close(sfd);
			#endif
			exit(EXIT_FAILURE);
		}
		
		buf_ptr += bytes_recv;
	}while(buf_ptr != BUF_SIZE);
	/* end receive file length */
	
	bytes_len_total = strtoull(filelength,NULL,10);
	
	/* log relative initialization */
	denominator = bytes_len_total;
	numerator = 0;
	resetlog(&next);
	/* end log relative initialization */

	printf("[client] file length received: %llu\n", bytes_len_total);

	/* receive file content */
	if(bytes_len_total != 0){

		while(bytes_len_total != 0){
			memset(buffer, 0, BUF_SIZE);
			bytes_write = (bytes_len_total >= BUF_SIZE ? BUF_SIZE : bytes_len_total);
			buf_ptr = 0;
		
			do{
				bytes_recv = recv(sfd, buffer + buf_ptr, bytes_write - buf_ptr, 0);
			
				if(bytes_recv < 0){
					perror("[error] on writing file: ");
					#ifdef LINUX
					close(sfd);
					#endif
					exit(EXIT_FAILURE);
				}

				buf_ptr += bytes_recv;
			}while(buf_ptr != bytes_write);
		
			buf_ptr = 0;

			do{
				buf_ptr += fwrite(buffer + buf_ptr, sizeof(char), bytes_write - buf_ptr, fptr);
					if(bytes_write < 0){
						perror("[error] on writing file: ");
						#ifdef LINUX
						close(sfd);
						#endif
						exit(EXIT_FAILURE);
					}
			}while(buf_ptr != bytes_write);

			bytes_len_total -= bytes_write;
			numerator += bytes_write;
			printlog(numerator, denominator,&next);
		}
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
