#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "my_defines.h"

/* Returns 2 integrities , diffrent combination from previous */
void get_integrities(int *table);

int  get_input_args(int argc, char *argv[], int *numOfSalads, int *mantime);

int main(int argc, char *argv[]){

    int retval, mantime, numOfSalads, num, id = 0, err = 0;
    Buffer *buffer;
    
    /* Get input arguments */
    if(get_input_args(argc, argv, &numOfSalads, &mantime) == -1){
        printf("Usage: -n <number_of_salads> -m <mantime> \n");
        return -1;
    }


    /* Make shared memory segment */
    id = shmget(IPC_PRIVATE , SEGMENTSIZE , SEGMENTPERM) ; 
    if(id == (void *) -1) perror(" Creation ");
    else printf("Allocated . %d\n" ,( int ) id);

    /* Attach the segment */
    buffer = (Buffer*) shmat(id ,( void *) 0, 0) ; 
    if (buffer == (void *) -1){ perror(" Attachment ."); exit(2);}
    buffer->n_salands = numOfSalads;

    /* Initialize the semaphores . */
    retval = sem_init (&buffer->chef,1 ,0) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit (3) ;}
    retval = sem_init (&buffer->test,1 ,0) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit (3) ;}
    

    /* Select random set of 2 n times */
    while(buffer->n_salands){
        printf("Chef picked: ");
        get_integrities(buffer->table);
        printf(" %s and %s \n", str_integrities[buffer->table[0]], str_integrities[buffer->table[1]]);
        printf("chef: n salads %d\n", --buffer->n_salands);
        sem_post(&buffer->test);
        retval = sem_wait(&buffer->chef);
    } 

    /* Remove segment */
    err = shmctl(id , IPC_RMID , 0);
    if (err == -1)perror(" Removal . ");
    else printf(" Removed . % d\n" ,( int )( err ));
    sem_destroy(&buffer->chef);

    return 0;

}


void get_integrities(int *table){

    int integrity;

    table[0] = rand() % 3;

    while((integrity = rand() % 3) ==  table[0] || integrity == table[1]);

    table[1] = integrity;


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