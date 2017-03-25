#ifndef UDP_H
#define UDP_H
#include "argsetup.h"

#define USEC_SLEEP (100)

extern void UDPS(char filename[], int sfd, struct addrinfo* send_info);
extern void UDPR(int sfd);

#endif
