#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "argsetup.h"
#include "UDP.h"
#include "ACK.h"

static unsigned long long int getFileSize(FILE* fptr);

extern void UDPS(char filename[], int cfd)
{

}

extern void UDPR(int sfd)
{

}

static unsigned long long int getFileSize(FILE* fptr)
{
	unsigned long long int fsize = 0;
	fseek(fptr, 0L, SEEK_END);
	fsize = ftell(fptr);
	rewind(fptr);
	return fsize;
}
