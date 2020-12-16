#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "my_defines.h"

/* Returns 2 integrities , diffrent combination from previous */
void get_integrities(int *table);

int  get_input_args(int argc, char *argv[], int *numOfSalads, int *mantime);

int get_cook_num(int *integrities);

void post_saladMakers(int exclude_saladMaker, sem_t *saladMakers);

void rest(int mantime){ usleep(1000 * mantime); }


int main(int argc, char *argv[]){

    int retval, mantime, numOfSalads, id = 0, err = 0;
    Buffer *buffer;
    
    srand(time(NULL));
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

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        retval = sem_init (&buffer->cooks[i],1 ,0) ;
        if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}
    }

    

    /* Select random set of 2 n times */
    int cook_num;
    while(buffer->n_salands){
        printf("chef: n salads %d\n", --buffer->n_salands);
        printf("Chef picked: ");
        
        //Select two integrities
        get_integrities(buffer->table); 
        
        //Find the proper salad maker, for the picked integrities 
        cook_num = get_cook_num(buffer->table);

        printf(" %s and %s \n", str_integrities[buffer->table[0]], str_integrities[buffer->table[1]]);
        printf("num:  %d\n", cook_num); 

        sem_post(&buffer->cooks[cook_num]);
        
        //wait him to take the integrities
        retval = sem_wait(&buffer->chef);
        rest(mantime);
    } 

    post_rest_saladMakers(cook_num, buffer->cooks);

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
    printf("test\n");
    for(int i = 0; i < N_SALAD_MAKERS; i++)
        if(i != exclude_saladMaker)
            sem_post(&saladMakers[i]);
}