#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "my_defines.h"
#include "time_utils.h"


int  get_input_args(int argc, char *argv[], int *numOfSalads, int *mantime);

/* Finds the proper saladmaker */
int get_cook_num(int *integrities);

/* Inform all salad makers that job is done */
void post_saladMakers(int exclude_saladMaker, sem_t *saladMakers);


void rest(int mantime){ usleep(1000 * mantime); }

/* Returns 2 integrities with different combination from previous */
void get_integrities(int *integrities);


void place_integrities(int *table, int *integrities)
{table[0] = integrities[0]; table[1] = integrities[1];}



typedef struct Record{
    char time[30], pid[10] ,msg_part[10];
    int cook_num;  
}Record;


/* Exctracts all the needed info from logfile */
void get_records(FILE *logfile, Record *records, int n_records);



int main(int argc, char *argv[]){

    srand(time(NULL));
    
    char timeStr[30], msg[100];
    struct timeval t2, elapsed;
    int retval, mantime, numOfSalads, id = 0, err = 0;

    Buffer *buffer;




    /* Get input arguments */
    if(get_input_args(argc, argv, &numOfSalads, &mantime) == -1){
        printf("Usage: -n <number_of_salads> -m <mantime> \n");
        return -1;
    }



    /* Create a logfile for all processes*/
    FILE *logfile;
    int pid = getpid();

    char filename[100];
    sprintf(filename, "logfile_%d.txt", pid);
    printf("%s\n", filename);
    logfile = fopen(filename, "a");
    srand(time(NULL));



    /* Make shared memory segment */
    id = shmget(IPC_PRIVATE , SEGMENTSIZE , SEGMENTPERM) ; 
    if(id == (void *) -1) perror(" Creation ");
    else printf("Allocated . %d\n" ,( int ) id);



    /* Attach the segment */
    buffer = (Buffer*) shmat(id ,( void *) 0, 0) ; 
    if (buffer == (void *) -1){ perror(" Attachment ."); exit(2);}



    /* Set values to the shared variables*/
    buffer->n_salands = numOfSalads;     
    sprintf(buffer->logfile, filename);   
    gettimeofday(&buffer->t1, NULL);     //Keep a global time for all processes
    buffer->table[0] = EMPTY;
    buffer->table[1] = EMPTY;

    for(int i = 0; i < N_SALAD_MAKERS; i++)
        buffer->saladsDone[i] = 0;



    /* Removes segment after the last detachment */
    err = shmctl(id , IPC_RMID , 0);
    if (err == -1)perror(" Removal . ");
    else printf(" Removed . % d\n" ,( int )( err ));
    


    /* Initialize the semaphores  */
    retval = sem_init (&buffer->chef,1 ,0) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit (3) ;}

    retval = sem_init (&buffer->log,1 ,1) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit (3) ;}

    retval = sem_init (&buffer->access_table,1 ,1) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit (3) ;}


    for(int i = 0; i < N_SALAD_MAKERS; i++){
        retval = sem_init (&buffer->cooks[i],1 ,0) ;
        if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}
    }



    /*** Start giving integrities to saladmakers ***/
    int cook_num, integrities[2];
    while(buffer->n_salands >= 0){
        
        /* Select two integrities */
        get_integrities(integrities);

        sprintf(msg, "Selecting integrities %s  %s"\
        ,str_integrities[integrities[0]], str_integrities[integrities[1]]);
        printf("%s\n", msg);
        fflush(stdout);

        //Writing in the logfile
        sem_wait(&buffer->log);  
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        fflush(logfile);

        sem_post(&buffer->log);
        //End of writing


        /* Find the proper salad maker, for the selected integrities */ 
        cook_num = get_cook_num(integrities);

        /* Place integrities in table */
        sem_wait(&buffer->access_table);
        place_integrities(buffer->table, integrities);
        sem_post(&buffer->access_table);

        /*  Notify the saladmaker */
        if(buffer->n_salands <= 0)break;
        sprintf(msg, "Notify saladmaker %d", cook_num);
        printf("%s\n", msg);
        fflush(stdout);


        //Writing in the logfile
        printf("about to write\n");
        sem_wait(&buffer->log);
        printf("wrote\n");
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        fflush(logfile);

        sem_post(&buffer->log);
        //End of writing


        if(buffer->n_salands <= 0)break;
        printf("about to wake cook\n");
        sem_post(&buffer->cooks[cook_num]);
        printf("woke cook\n");

        
        //wait salad maker to take the integrities
        if(buffer->n_salands <= 0)break;
        retval = sem_wait(&buffer->chef);
        printf("remaining salads %d\n", buffer->n_salands);
        fflush(stdout);

        sprintf(msg, "Man time for resting");
        printf("%s\n", msg);
        
        //Writing to logfile
        sem_wait(&buffer->log);

        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        fflush(logfile);
        
        sem_post(&buffer->log);

        if(buffer->n_salands <= 0)break;
        rest(mantime);
    } 

    buffer->table[0] = EMPTY;
    buffer->table[1] = EMPTY;


    printf("Inform salad makers that job is done \n");
    post_saladMakers(cook_num, buffer->cooks);



    /*** Stdout of chef: ***/

    printf("\nTotal #salads: %d\n", numOfSalads - buffer->n_salands);

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        printf("salads of salad_maker%d [%d]: %d\n", \
        i, buffer->cooks_pid[i], buffer->saladsDone[i]);
    }

    /* Print a list with all concured processes */
    printf("\nTime intervals: (in inscreasing order)\n");
    fclose(logfile);
    fopen(filename, "r");
    char ch, time_interval[100], pids[100];

    int lines = 0;

    //find number of lines
    while((ch = fgetc(logfile)) != EOF)
        if(ch == '\n')lines++;

    rewind(logfile);
    
    Record records[lines];
    get_records(logfile, records, lines);

    int concured_pid[N_SALAD_MAKERS], concured_counter = 0, start = -1;

    for(int i = 0; i < N_SALAD_MAKERS; i++)concured_pid[i] = 0;

    /* Find concured processes */
    for(int r = 0, i = 0; r < lines; r++){
        //printf("%s\n", records[r].msg_part);

        /* start of time interval */
        if(!strcmp(records[r].msg_part, "Get")){
            if(start == -1)
                start = r;              
            concured_pid[records[r].cook_num]++;

        }
        else if(!strcmp(records[r].msg_part, "Start"))
            concured_pid[records[r].cook_num]++;

        /* End of time interval, search if 2=< processed run concurrently */
        else if(!strcmp(records[r].msg_part, "End")){
            
            for(int j = 0; j < N_SALAD_MAKERS; j++)
                if(concured_pid[j])concured_counter++;

            //return 0;
            //Print start - end time
            if(concured_counter >= 2 && start != -1){
                printf("[%s - %s]", records[start].time, records[r].time );

                //Print saladmakers that worked concurrently 
                for(int j = 0; j < N_SALAD_MAKERS; j++)
                    if(concured_pid[j])printf(" saladmaker%d ", j);

                printf("\n");
            }

            concured_counter = 0;
            concured_pid[records[r].cook_num] = 0;
            start =  -1;
        }

        

    }



    sem_destroy(&buffer->chef);

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        retval = sem_destroy(&buffer->cooks[i]) ;
        if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}
    }


    fclose(logfile);
    return 0;

}


void get_integrities(int *integrities){

    int integrity;

    integrities[0] = rand() % 3;

    while((integrity = rand() % 3) ==  integrities[0] || integrity == integrities[1]);

    integrities[1] = integrity;


}


int  get_input_args(int argc, char *argv[], int *numOfSalads, int *mantime){
    
    if(argc == 5){
        for(int i = 1; i < argc-1; i++){
            
            if(!strcmp(argv[i],"-n"))    
                *numOfSalads = atoi(argv[i+1]);
            
            else if(!strcmp(argv[i],"-m"))
                *mantime = atoi(argv[i+1]);
                
        }
        return 0;
    }

    return -1;
}



void get_records(FILE *logfile, Record *records, int n_records){
    
    int i, j, flag, len = 200;
    char line[len];

    int r = 0;

    while (fgets(&line, len,  logfile) != NULL){
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
        if(getpid() != atoi(records[r].pid))
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



int get_cook_num(int *integrities){

    int cook_num;

    if(tomato != integrities[0] && tomato != integrities[1])
        cook_num = tomato;
    else if(green_pepper != integrities[0] && green_pepper != integrities[1])
        cook_num = green_pepper;
    else cook_num = small_onion;

    return cook_num;

}


void post_saladMakers(int exclude_saladMaker, sem_t *saladMakers){
    for(int i = 0; i < N_SALAD_MAKERS; i++)
        //if(i != exclude_saladMaker)
        sem_post(&saladMakers[i]);
}