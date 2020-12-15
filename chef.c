#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include "my_defines.h"

/* Returns 2 integrities , diffrent combination from previous */
void get_integrities(int *table);

int main(int argc, char *argv[]){
    int retval, id = 0, err = 0;
    Buffer *buffer;

    /* Get input arguments */
    if(argc != 2){
        printf("Usage: <number_of_salads>\n");
        return -1;
    }


    /* Make shared memory segment */
    id = shmget(IPC_PRIVATE , SEGMENTSIZE , SEGMENTPERM) ; 
    if(id == (void *) -1) perror(" Creation ");
    else printf("Allocated . %d\n" ,( int ) id);

    /* Attach the segment */
    buffer = (Buffer*) shmat(id ,( void *) 0, 0) ; 
    if (buffer == (void *) -1){ perror(" Attachment ."); exit(2);}
    buffer->n_salands = atoi(argv[1]);

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
        sem_post(&buffer->test);
        printf("chef: n salads %d\n", --buffer->n_salands);
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
