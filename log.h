#ifndef LOG_H
#define LOG_H

/************************************
 *	Define log step and range		*
 *	current range: 0 ~ 100			*
 ***********************************/
#define LOG_STEP ((unsigned long long int)5)
#define LOG_MIN ((unsigned long long int)0)
#define LOG_MAX ((unsigned long long int)100)
#define LOG_STRLEN_MAX ((unsigned long long int)100)

extern void printlog(unsigned long long int numerator, unsigned long long int denominator, unsigned long long int* next);
extern void resetlog(unsigned long long int* next);

#endif
