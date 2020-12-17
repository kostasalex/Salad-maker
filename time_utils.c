
#include <stdio.h>
#include "time_utils.h"

void time_elapsed(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
  if (t2->tv_usec < t1->tv_usec) {
    int nsec = (t1->tv_usec - t2->tv_usec) / 1000000 + 1;
    t1->tv_usec -= 1000000 * nsec;
    t1->tv_sec += nsec;
  }

  if (t2->tv_usec - t1->tv_usec > 1000000) {
    int nsec = (t2->tv_usec - t1->tv_usec) / 1000000;
    t1->tv_usec += 1000000 * nsec;
    t1->tv_sec -= nsec;
  }

  result->tv_sec = t2->tv_sec - t1->tv_sec;
  result->tv_usec = t2->tv_usec - t1->tv_usec;

}

void time_to_string(struct timeval t, char *str_time){

    time_t curtime; curtime = t.tv_sec;
    strftime(str_time,30,"%T",localtime(&curtime));
    sprintf(str_time, "%s:%02.2f",str_time,(float)t.tv_usec/1000);

}