#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>  /* times() */
#include "my_defines.h"

int
timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating @var{y}. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     @code{tv_usec} is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

int  get_input_args(int argc, char *argv[], int *lb, int *ub,
int *id, int *cook_num);

/* Making a salad in random time between lb and ub */
void make_salad(int lb, int ub);   

int main ( int argc , char ** argv )
{
    int retval, id , err , lb, ub, cook_num;
    Buffer *buffer;

    struct timeval t1, t2, elapsed;


    /* Get input arguments */
    if(get_input_args(argc, argv, &lb, &ub, &id, &cook_num) == -1){
        printf("Usage: -t1 <lb> -t2 <lu> -s <shmid> -c <cook_num> \n");
        return -1;
    }


    FILE *logfile, *global_log;
    int pid = getpid();

    char filename[100];
    sprintf(filename, "salad_maker%d_%d.txt", cook_num, pid);
    printf("%s\n", filename);
    logfile = fopen(filename, "a");

    srand(time(NULL));


    /* Get id from command line . */
    sscanf ( argv [1] , "%d" , & id );
    printf ( " Allocated %d \n" , id ) ;

    /* Attach the segment . */
    buffer = (Buffer *)shmat(id ,( void *) 0, 0);
    if (buffer == ( void *) -1) { perror (" Attachment ."); exit (2) ;}
    global_log = fopen(buffer->logfile, "a");

    gettimeofday(&t1, NULL);

    char timestr[30];
    
    /* Take integrities from chef(read) , write in log files */
    while(buffer->n_salands){

        //Wait for chef to give integrities
        fprintf(logfile,"salad_maker%d Waiting for integrities\n", cook_num);
        sem_wait(&buffer->cooks[cook_num]);

        fprintf(logfile,"salad_maker%d get %s and %s\n", cook_num, \
        str_integrities[buffer->table[0]], str_integrities[buffer->table[1]]);
       
        
        //Inform chef that integrities received
        sem_post(&buffer->chef);
        gettimeofday(&t2, NULL);
        
        timeval_subtract(&elapsed, &t2, &t1);
        
        fprintf(logfile, "%02ld:%02ld:%.2f",elapsed.tv_sec/60, elapsed.tv_sec % 60, (float)elapsed.tv_usec/1000);

        fprintf(logfile,"salad_maker%d start making salad\n", cook_num);
        make_salad(lb,ub);
        fprintf(logfile,"salad_maker%d end making salad\n", cook_num);     
       
        fprintf(logfile,"salad_maker: n salads %d\n", buffer->n_salands);
    } 
    
    
    /* Remove segment . */
    err = shmctl (id , IPC_RMID , 0) ;
    if (err == -1) perror (" Removal . ");
    else printf (" Removed . % d\n" , ( int )( err )) ;
    sem_destroy (&buffer->chef);

    fclose(logfile);
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



void make_salad(int lb, int ub){     
    int worktime, timeRange = lb-ub;
    if(timeRange > 0)worktime = (rand() % (lb-ub)) + ub;
    else worktime = 0;
    usleep(worktime * 1000);
}