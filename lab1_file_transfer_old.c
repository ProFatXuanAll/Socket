#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "argsetup.h"

/* Linux socket library include goes here */
#ifdef LINUX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

/* Microsoft Windows socket library include goes here */
#ifdef MSDOS

#include <winsock.h>

#endif

static int CheckARGV(char* argv[]);
static void ServerStartUp(char Protocol[],char IP[],char Port[],char Filename[]);
static void ClientStartUp(char Protocol[],char IP[],char Port[],char Filename[]);
static unsigned long long int getFileSize(FILE *fptr);
static void Error(char mesg[]);
static void TCPS(char filename[],int cfd);
static void TCPR(int sfd);
static void UDPS(char filename[],int cfd);
static void UDPR(int sfd);
static char ACK[]=ACK_ENCODE;
static void ACK_Set(unsigned long long int* num,char dst[],const char src[],unsigned long long int len);
static int ACK_Check(unsigned long long int* num,char src[],unsigned long long int* len);

int main(int argc, char* argv[])
{
	if(argc<ARGC_MIN){	/* main function command line argument error check */
		Error("usage: ./server <protocol> <action> <ip> <port> [<file>]");
	}
	
	if(CheckARGV(argv)){
		Error("[error] argument format error");
	}
	
	if(tolower(argv[2][0])=='s'){	/* send means server */
		if(argc!=6){
			Error("[error] <file> input missing.");
		}
		ServerStartUp(argv[1],argv[3],argv[4],argv[5]);
	}
	else{	/* receive means client */
		ClientStartUp(argv[1],argv[3],argv[4],NULL);
	}

	return 0;
}

static void ServerStartUp(char Protocol[],char IP[],char Port[],char Filename[])
{
	int serv_sock_fd;					/* server socket file descriptor */
	int clie_sock_fd;					/* client socket file descriptor */
	int status;							/* function return status */
	struct addrinfo hints;				/* helper for constructing server information */
	struct addrinfo* serv_info;			/* server socket information */
	struct sockaddr_storage clie_info;	/* client socket information */
	socklen_t addrlen;					/* client socket information size */

	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;												/* either IPv4 or IPv6 */
	hints.ai_socktype=(tolower(Protocol[0])=='t'?SOCK_STREAM:SOCK_DGRAM);	/* TCP or UDP socket stream */
	hints.ai_flags=AI_PASSIVE;												/* automatically fill in IP */

	status=getaddrinfo(IP,Port,&hints,&serv_info);

	if(status!=0){	/* getaddrinfo function error check */
		Error("[error] on getaddrinfo: ");
	}

	serv_sock_fd=socket(serv_info->ai_family,serv_info->ai_socktype,serv_info->ai_protocol);

	if(serv_sock_fd<0){	/* socket function error check */
		Error("[error] on open socket: ");
	}

printf("[server debug] before bind\n");

	status=bind(serv_sock_fd,serv_info->ai_addr,serv_info->ai_addrlen);

	if(status<0){	/* bind function error check */
		Error("[error] on bind: ");
	}

printf("[server debug] after bind\n");
printf("[server debug] before listen\n");

	status=listen(serv_sock_fd,BACKLOG);

	if(status<0){	/* listen function error check */
		Error("[error] on listen: ");
	}

printf("[server debug] after listen\n");
printf("[server debug] before accept\n");

	clie_sock_fd=accept(serv_sock_fd,(struct sockaddr*)&clie_info,&addrlen);

	if(clie_sock_fd<0){	/* accept function error check */
		Error("[error] on accept: ");
	}

printf("[server debug] after accept\n");
	/* Display the client's address */
	if(clie_info.ss_family==AF_INET){	/* IPv4 version */
		printf("[client] IPv4");
		if(inet_ntop(AF_INET,(struct sockaddr_in*)&clie_info,IP,strlen(IP))!=NULL){
			printf("Client address is %s\n",IP);
			printf("Client port is %d\n",ntohs(((struct sockaddr_in*)&clie_info)->sin_port));
		}
		else{
			Error("[error] on convert IPv4 address: ");
		}
	}
	else{	/* IPv6 version */
		if(inet_ntop(AF_INET6,(struct sockaddr_in6*)&clie_info,IP,strlen(IP))!=NULL){
			printf("Client address is %s\n",IP);
			printf("Client port is %d\n",ntohs(((struct sockaddr_in6*)&clie_info)->sin6_port));
		}
		else{
			Error("[error] on convert IPv6 address: ");
		}
	}

	if(tolower(Protocol[0])=='t'){	/* TCP:send */
		TCPS(Filename,clie_sock_fd);
	}
	else{	/* UDP:receive */
		UDPS(Filename,clie_sock_fd);
	}

	#ifdef LINUX
	status=close(clie_sock_fd);
	#endif

	#ifdef MSDOS

	#endif

	if(status<0){
		Error("[error] on client socket close: ");
	}

	#ifdef LINUX
	status=close(serv_sock_fd);
	#endif

	#ifdef MSDOS

	#endif

	if(status<0){
		Error("[error] on server socket close: ");
	}

	freeaddrinfo(serv_info);
}

void ClientStartUp(char Protocol[],char IP[],char Port[],char Filename[])
{
	int serv_sock_fd;	/* server socket file descriptor */
	struct addrinfo hints;	/* getaddrinfo setup */
	struct addrinfo *serv_info;	/* server address information */
	int status;
	struct addrinfo *p;
	void *addr;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;	/* either IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;	/* TCP stream socket */
	hints.ai_flags = AI_PASSIVE;	/* automatically fill in IP address */

	status = getaddrinfo(IP,Port,&hints,&serv_info);

	if(status!=0){	/* getaddrinfo function error check */
		Error("[error] getaddrinfo error: ");
	}

	/* Display the server's address */
	for(p=serv_info;p!=NULL;p=p->ai_next){
		if(p->ai_family==AF_INET){
			addr=&(((struct sockaddr_in*)p->ai_addr)->sin_addr);
		}
		else if(p->ai_family==AF_INET6){
			addr=&(((struct sockaddr_in6*)p->ai_addr)->sin6_addr);
		}

		inet_ntop(p->ai_family,addr,IP,strlen(IP));
		printf("[client] server address is %s\n",IP);
	}

	serv_sock_fd=socket(serv_info->ai_family,serv_info->ai_socktype,serv_info->ai_protocol);

	if(serv_sock_fd<0){	/* socket function error check */
		freeaddrinfo(serv_info);
		Error("[error] on open socket: ");
	}

	status=connect(serv_sock_fd,serv_info->ai_addr,serv_info->ai_addrlen);

	if(status<0){	/* connect function error check */
		freeaddrinfo(serv_info);
		Error("[error] on connect: ");
	}
	
	if(tolower(Protocol[0])=='t'){	/* TCP:receive */
		TCPR(serv_sock_fd);
	}
	else{	/* UDP:receive */
		UDPR(serv_sock_fd);
	}

	#ifdef LINUX
	status=close(serv_sock_fd);
	#endif

	#ifdef MSDOS

	#endif

	if(status<0){
		freeaddrinfo(serv_info);
		Error("[error] on client socket close: ");
	}

	freeaddrinfo(serv_info);
	return;
}


static int CheckARGV(char* argv[])
{
	struct sockaddr_storage test;

	if(strcmp(argv[1],"tcp")!=0&&strcmp(argv[1],"TCP")!=0&&
	strcmp(argv[1],"udp")!=0&&strcmp(argv[1],"UDP")!=0){
		fprintf(stderr,"[error] <protocol> options: tcp | TCP | udp | UDP\n");
		return -1;
	}
	else if(strcmp(argv[2],"send")!=0&&strcmp(argv[2],"recv")!=0){
		fprintf(stderr,"[error] <action> options: send | recv\n");
		return -1;
	}
	else if(inet_pton(AF_INET,argv[3],&test)<=0&&inet_pton(AF_INET6,argv[3],&test)<=0){
		fprintf(stderr,"[error] invalid <ip> format\n");
		return -1;
	}
	else if(atoi(argv[4])<0||atoi(argv[4])>65536){
		fprintf(stderr,"[error] <port> range: 0~65535\n");
		return -1;
	}
	else{
		return 0;
	}
}

static unsigned long long int getFileSize(FILE *fptr)
{
	unsigned long long int fsize=0;
	fseek(fptr,0L,SEEK_END);
	fsize=ftell(fptr);
	rewind(fptr);
	return fsize;
}

static void Error(char mesg[])
{
	if(errno){
		perror(mesg);
	}
	else{
		fprintf(stderr,"%s\n",mesg);
	}
	exit(EXIT_FAILURE);
}

static void TCPS(char filename[],int cfd)
{
	FILE *fptr;
	char buffer[BUF_SIZE], fbuffer[BUF_SIZE];
	unsigned long long int bytes_send, bytes_send_total, bytes_left;
	unsigned long long int ACK_counter;
	unsigned long long int ACK_len;
	int flag;

	fptr=fopen(filename,"rb");

	if(fptr==NULL){	/* fopen function error check */
		Error("[error] file open failure: ");
	}

	ACK_counter=0;	/* IMPORTANT!!! */
	flag=1;	/* IMPORTANT!!! */

	/* send file name */
	bytes_left=strlen(filename);
	bytes_send_total=0;
	do{
		if(bytes_left>=BUF_SIZE-ACK_LEN){
			ACK_Set(&ACK_counter,buffer,filename+bytes_send_total,BUF_SIZE-ACK_LEN);
		}
		else{
			ACK_Set(&ACK_counter,buffer,filename+bytes_send_total,bytes_left);
			flag=0;
			/* may need padding bits */
		}
		
		bytes_send=send(cfd,buffer,BUF_SIZE,0);
		
		if(bytes_send<0){
			Error("[error] on sendind filename: ");
		}

		bytes_left-=(bytes_send-ACK_LEN);
		bytes_send_total+=(bytes_send-ACK_LEN);
	}while(bytes_left>0&&flag!=0);
	/* end send file name */

	/* special case when equal size, ACK for sending filename */
	if(flag){
		ACK_Set(&ACK_counter,buffer,"",0);
		bytes_send=send(cfd,buffer,BUF_SIZE,0);
	
		if(bytes_send<0){
			Error("[error] on sendind filename: ");
		}
	}
	/* end ACK for sending filename */
#ifdef DEBUG	
	flag=1;
	/* send file content */
	while(!feof(fptr)){
		bytes_left=fread(fbuffer,sizeof(char),BUF_SIZE-ACK_LEN,fptr);
		bytes_send_total=0;
		do{
			if(bytes_left>=BUF_SIZE-ACK_LEN){
				ACK_Set(&ACK_counter,buffer,fbuffer,BUF_SIZE-ACK_LEN);
			}
			else{
				ACK_Set(&ACK_counter,buffer,fbuffer,bytes_left);
				flag=0;
				/* may need padding bits */
			}

			bytes_send=send(cfd,buffer,BUF_SIZE,0);
			
			if(bytes_send<0){
				Error("[error] on sending file content: ");
			}

			bytes_left-=(bytes_send-ACK_LEN);
			bytes_send_total+=(bytes_send-ACK_LEN);
		}while(bytes_left>0&&flag!=0);
	}
	
	/* special case when equal size, ACK for sending file content */
	if(flag){
		ACK_Set(&ACK_counter,buffer,"",0);
		bytes_send=send(cfd,buffer,BUF_SIZE,0);
	
		if(bytes_send<0){
			Error("[error] on sendind filename: ");
		}
	}
	/* end ACK for sending file content */

#endif
	fclose(fptr);
	/* end send file connent */
}

static void TCPR(int sfd)
{
	FILE *fptr;
	char buffer[BUF_SIZE];
	char filename[NAME_SIZE_MAX];
	unsigned long long int bytes_recv, bytes_recv_total, bytes_write;
	unsigned long long int ACK_counter;
	int flag;

	/* receive file name first */
	bytes_recv_total=0;
	while(1){
		if((recv(sfd,buffer,sizeof(buffer),0))<0){
			Error("[error] on receiving filename: ");
		}
		if(ACK_Check(&ACK_counter,buffer,&bytes_recv)<0){
			Error("[error] on filename ACK check failed.");
		}
		if(bytes_recv<BUF_SIZE-ACK_LEN){	/* ACK inform filename transfer end */
			strncpy(filename+bytes_recv_total,buffer,bytes_recv);
			bytes_recv_total+=bytes_recv;
			filename[bytes_recv_total]='\0';
			break;
		}
		strncpy(filename+bytes_recv_total,buffer,bytes_recv);
		bytes_recv_total+=bytes_recv;
	}
	/* end receive file name */
	
	printf("[client] filename received: %s\n",filename);

	fptr=fopen(filename,"wb");
	
	if(fptr==NULL){	/* fopen function error check */
		Error("[error] file open failure: ");
	}

#ifdef DEBUG
	/* receive file content */
	flag=1;
	bytes_recv_total=0;
	while(flag){
		if(bytes_recv=recv(sfd,buffer,sizeof(buffer),0)<0){
			Error("[error] on writing file: ");
		}
		if(ACK_Check(&ACK_counter,buffer,&bytes_recv)<0){
			Error("[error] on file ACK check failed.");
		}
		if(bytes_recv<BUF_SIZE-ACK_LEN){	/* ACK inform file transfer end */
			if(bytes_recv==0){
				break;	/* ACK end of transfer indicator */
			}
			else{
				flag=0;
			}
		}
		bytes_write=fwrite(buffer,sizeof(char),bytes_recv,fptr);
		if(bytes_write<0){
			Error("[error] on writing file: ");
		}
		bytes_recv_total+=bytes_recv;
	}
	/* end receive file content */

	printf("[client] file content received.\n");
	
#endif
	fclose(fptr);
	/* end receive file connent */
}

static void UDPS(char filename[],int cfd)
{
}

static void UDPR(int sfd)
{

}

static void ACK_Set(unsigned long long int* num,char dst[],const char src[],unsigned long long int len){
	unsigned long long ACK_len;

	sprintf(dst,"%c%.20llu%c%.5llu%c",ACK[0],*num,ACK[ACK_NUM_MAX+1],len,ACK[ACK_LEN-1]);
	ACK_len=strlen(dst);
	strncpy(dst+ACK_len,src,len);
	(*num)++;
}

static int ACK_Check(unsigned long long int* num,char src[],unsigned long long int* len)
{
	char tmp[BUF_SIZE];
	char tmp2,tmp3,tmp4;
	strncpy(tmp,src,ACK_LEN);
	tmp[ACK_LEN]='\0';
	
	if(sscanf(tmp,"%c%llu%c%llu%c",&tmp2,num,&tmp3,len,&tmp4)!=5){
		return -1;
	}
	else if(tmp2!=ACK[0]||tmp3!=ACK[ACK_NUM_MAX+1]||tmp4!=ACK[ACK_LEN-1]){
		return -1;
	}
	else{
		strncpy(tmp,src+ACK_LEN,*len);
		strncpy(src,tmp,*len);
		src[*len]='\0';
	}

	return 0;
}
