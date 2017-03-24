#ifndef SERVEROS_H
#define SERVEROS_H
	
/********************************* 
 * define which OS you are using *
 * options: LINUX, MSDOS         *
 ********************************/
#define LINUX

/********************************* 
 * server IP you are using       *
 ********************************/
#define SERV_IP "127.0.0.1"

/********************************* 
 * input argument counter max    *
 ********************************/
#define ARGC_MIN (5)

/********************************* 
 * server listening queue size   *
 ********************************/
#define BACKLOG (20)

/********************************* 
 * server send/recv buffer size  *
 ********************************/
#define BUF_SIZE ((unsigned long long int)1024)

/********************************* 
 * client file name buffer size  *
 ********************************/
#define NAME_SIZE_MAX (1024)

/********************************* 
 * ACK encode format, x=[0-9]*   *
 ********************************/
#define ACK_ENCODE "AxxxxooooxxxxooooxxxxCxxxxxK"
#define ACK_LEN ((unsigned long long int)strlen(ACK_ENCODE))
#define ACK_NUM_MAX (20)
#define ACK_INIT (0)
#define ACK_CNCA (1)

#endif
