#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>  

/* Returns current time in a given string */
void time_to_string(char *str_time);

void write_messages(char *stdout_msg, char *file_msg, FILE *a, FILE *b);

typedef struct Record{
    char time[30], pid[10] ,msg_part[10];
    int cook_num;  
}Record;

/* Extracts all the needed info from logfile */
void get_records(FILE *logfile, Record *records, int n_records, int pid);

/* Returns the total working processes */
int count_concurent(int *working_pid, int n_pids);