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

int get_cook_num(int *integrities);

void post_saladMakers(int exclude_saladMaker, sem_t *saladMakers);

void rest(int mantime){ usleep(1000 * mantime); }

/* Returns 2 integrities with different combination from previous */
void get_integrities(int *integrities);

void place_integrities(int *table, int *integrities)
{table[0] = integrities[0]; table[1] = integrities[1];}

int main(int argc, char *argv[]){

    char timeStr[30], msg[100];
    struct timeval t2, elapsed;
    
    int retval, mantime, numOfSalads, id = 0, err = 0;
    Buffer *buffer;

    srand(time(NULL));
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

    /* Remove segment after the last detachment */
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


    /* Give 2 integrities(write) to proper salad maker*/
    int cook_num, integrities[2];

    while(buffer->n_salands--){
        
        /* Select two integrities */
        get_integrities(integrities);

        sprintf(msg, "Selecting integrities %s  %s"\
        ,str_integrities[integrities[0]], str_integrities[integrities[1]]);
        printf("%s\n", msg);

        //Writing to logfile
        sem_wait(&buffer->log);
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        
        sem_post(&buffer->log);



        /* Find the proper salad maker, for the selected integrities */ 
        cook_num = get_cook_num(integrities);

        /* Place integrities in table */
        sem_wait(&buffer->access_table);
        place_integrities(buffer->table, integrities);
        sem_post(&buffer->access_table);

        /*  Notify the saladmaker */
        sprintf(msg, "Notify saladmaker %d", cook_num);
        printf("%s\n", msg);


        //Writing to logfile
        sem_wait(&buffer->log);

        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);

        sem_post(&buffer->log);

        sem_post(&buffer->cooks[cook_num]);


        
        //wait salad maker to take the integrities
        retval = sem_wait(&buffer->chef);
        printf("remaining salads %d\n", buffer->n_salands);

        sprintf(msg, "Man time for resting");
        printf("%s\n", msg);
        
        //Writing to logfile
        sem_wait(&buffer->log);

        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(logfile, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);

        sem_post(&buffer->log);

        rest(mantime);
    } 

    buffer->table[0] = EMPTY;
    buffer->table[1] = EMPTY;


    printf("Inform salad makers that job is done \n");
    post_saladMakers(cook_num, buffer->cooks);

    
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