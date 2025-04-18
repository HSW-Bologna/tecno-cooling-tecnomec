#ifndef SYSTEM_TIME_H_INCLUDED
#define SYSTEM_TIME_H_INCLUDED


#include <sys/time.h>
#include <time.h>


struct tm system_time_get(void);
void      system_time_set(struct tm *systm);


#endif
