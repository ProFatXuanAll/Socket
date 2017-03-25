#ifndef UDP_H
#define UDP_H
#include "argsetup.h"

#define USEC_SLEEP (500)

extern void UDPS(char filename[], int cfd, struct addrinfo* send_info);
extern void UDPR(int sfd);

#endif
