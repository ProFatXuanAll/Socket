#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argsetup.h"
#include "ACK.h"

extern void ACK_Set(unsigned long long int* num, char dst[], const char src[], unsigned long long int len){
	unsigned long long ACK_len;

	sprintf(dst, "%c%.20llu%c%.5llu%c", ACK[0], *num, ACK[ACK_NUM_MAX + 1], len, ACK[ACK_LEN - 1]);
	ACK_len = strlen(dst);
	strncpy(dst + ACK_len, src, len);
	(*num)++;
}

extern int ACK_Check(unsigned long long int* num, char src[], unsigned long long int* len)
{
	char tmp[BUF_SIZE];
	char tmp2, tmp3, tmp4;
	strncpy(tmp, src, ACK_LEN);
	tmp[ACK_LEN] = '\0';
	
	if(sscanf(tmp, "%c%llu%c%llu%c", &tmp2, num, &tmp3, len, &tmp4) != 5){
		return -1;
	}
	else if(tmp2 != ACK[0] || tmp3 != ACK[ACK_NUM_MAX + 1] || tmp4 != ACK[ACK_LEN - 1]){
		return -1;
	}
	else{
		strncpy(tmp, src + ACK_LEN, *len);
		strncpy(src, tmp, *len);
		src[*len] = '\0';
	}

	return 0;
}
