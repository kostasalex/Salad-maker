
#include <stdio.h>
#include "utils.h"


void time_to_string(char *str_time){
    
    struct timeval t;

    gettimeofday(&t, NULL);
    time_t curtime; curtime = t.tv_sec;

    strftime(str_time,30,"%T",localtime(&curtime));
    sprintf(str_time, "%s:%02.2f",str_time,(float)t.tv_usec/1000);

}

void write_messages(char *stdout_msg, char *file_msg, FILE *a, FILE *b){
  
  printf("%s\n", stdout_msg);
  fflush(stdout);
  
  if(a){
    fprintf(a, "%s", file_msg);
    fflush(a);
  }
  if(b){
    fprintf(b, "%s", file_msg);
    fflush(b);
  }

}

void get_records(FILE *logfile, Record *records, int n_records, int pid){
    
    int i, j, flag, len = 200;
    char line[len];

    int r = 0;

    while (fgets(line, len,  logfile) != NULL){
        i = 0;
        j = 0;

        while(line[i] != ']'){
            if(line[i] != '[')
                records[r].time[j++] = line[i];
            i++;
        }
        records[r].time[j] = '\0';i++;//skip space


        j = 0;
        while(line[i] != ']'){
            if(line[i] != '[')
                records[r].pid[j++] = line[i];
            i++;
        }
        records[r].pid[j] = '\0';i++;//skip space


        
        while(line[i++] != ']');

        //if not chef , save salad maker's number
        if(pid != atoi(records[r].pid))
            records[r].cook_num = line[i-2] - '0';
        else records[r].cook_num = -1;

            
        
        i++;//skip space

        j = 0;
        flag = 0;
        while(line[i] != ']'){
            if(line[i] == ' ')flag = 1;
            if(line[i] != '[' && !flag)records[r].msg_part[j++] = line[i];
            i++;
        }

        records[r].msg_part[j] = '\0';

        r++;
    }


}


int count_concurent(int *pid, int n_pids){

    int concured_counter = 0;

    for(int i = 0; i < n_pids; i++)
        if(pid[i])concured_counter++;

    return concured_counter;
}