#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "my_defines.h"


int main ( int argc , char ** argv )
{
    int retval, id , err ;
    Buffer *buffer;

    if ( argc <= 1) { printf (" Need shmem id . \n "); exit (1) ; }
    /* Get id from command line . */
    sscanf ( argv [1] , "%d" , & id );
    printf ( " Allocated %d \n" , id ) ;

    /* Attach the segment . */
    buffer = (Buffer *)shmat(id ,( void *) 0, 0);
    if (buffer == ( void *) -1) { perror (" Attachment ."); exit (2) ;}

    /* Select random set of 2 n times */
    while(buffer->n_salands){
        sem_wait(&buffer->test);
        printf("Cook got: ");
        sem_post(&buffer->chef);
        printf(" %s and %s \n", str_integrities[buffer->table[0]], str_integrities[buffer->table[1]]);
        printf("cook: n salads %d\n", buffer->n_salands);
    } 
    
    
    /* Remove segment . */
    err = shmctl (id , IPC_RMID , 0) ;
    if (err == -1) perror (" Removal . ");
    else printf (" Removed . % d\n" , ( int )( err )) ;
    sem_destroy (&buffer->chef);

    return 0;

}