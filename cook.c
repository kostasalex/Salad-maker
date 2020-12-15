#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "my_defines.h"

int  get_input_args(int argc, char *argv[], int *lb, int *ub,
int *id, int *cook_num);

int main ( int argc , char ** argv )
{
    int retval, id , err , lb, ub, cook_num;
    Buffer *buffer;

    /* Get input arguments */
    if(get_input_args(argc, argv, &lb, &ub, &id, &cook_num) == -1){
        printf("Usage: -n <number_of_salads> -m <mantime> \n");
        return -1;
    }

    /* Get id from command line . */
    sscanf ( argv [1] , "%d" , & id );
    printf ( " Allocated %d \n" , id ) ;

    /* Attach the segment . */
    buffer = (Buffer *)shmat(id ,( void *) 0, 0);
    if (buffer == ( void *) -1) { perror (" Attachment ."); exit (2) ;}

    /* Select random set of 2 n times */
    while(buffer->n_salands){
        sem_wait(&buffer->test);
        //printf("Cook got: ");
        sem_post(&buffer->chef);
        //printf(" %s and %s \n", str_integrities[buffer->table[0]], str_integrities[buffer->table[1]]);
        printf("cook: n salads %d\n", buffer->n_salands);
    } 
    
    
    /* Remove segment . */
    err = shmctl (id , IPC_RMID , 0) ;
    if (err == -1) perror (" Removal . ");
    else printf (" Removed . % d\n" , ( int )( err )) ;
    sem_destroy (&buffer->chef);

    return 0;

}

int  get_input_args(int argc, char *argv[], int *lb, int *ub,
int *id, int *cook_num){
    
    if(argc == 9){
        for(int i = 1; i < argc-1; i++){
            
            if(!strcmp(argv[i],"-t1"))    
                *lb = atoi(argv[i+1]);
            
            else if(!strcmp(argv[i],"-t2"))
                *ub = atoi(argv[i+1]);
                
            else if(!strcmp(argv[i],"-s"))
                *id = atoi(argv[i+1]);
            
            else if(!strcmp(argv[i],"-c"))
                *cook_num = atoi(argv[i+1]);

        }
        return 0;
    }

    return -1;
}