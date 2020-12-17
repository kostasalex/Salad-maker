#include <time.h>
#include <sys/time.h>  

void time_elapsed(struct timeval *result,\
struct timeval *t2, struct timeval *t1);

void time_to_string(struct timeval t, char *str_time);