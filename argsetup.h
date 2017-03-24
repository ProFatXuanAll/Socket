#ifndef ARGSETUP_H
#define ARGSETUP_H
	
/************************************
 *	define which OS you are using	*
 *	options: LINUX, MSDOS			*
 ***********************************/
#define LINUX

/************************************
 *	Linux socket library includes	*
 *	goes here						*
 ***********************************/
#ifdef LINUX

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#endif

/************************************
 *	Microsoft Windows socket 		*
 *	library includes goes here 		*
 ***********************************/
#ifdef MSDOS

#include <winsock.h>

#endif


/************************************ 
 *	server IP you are using       	*
 ***********************************/
#define SERV_IP "127.0.0.1"

/************************************
 *	input argument counter max		*
 ***********************************/
#define ARGC_MIN (5)

/************************************
 *	server listening queue size		*
 ***********************************/
#define BACKLOG (20)

/************************************ 
 *	server send/recv buffer size	*
 ***********************************/
#define BUF_SIZE ((unsigned long long int)1024)

/************************************ 
 *	client file name buffer size	*
 ***********************************/
#define NAME_SIZE_MAX (1024)

#endif
