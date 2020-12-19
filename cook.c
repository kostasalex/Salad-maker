#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>  
#include "my_defines.h"
#include "time_utils.h"


int  get_input_args(int argc, char *argv[], int *lb, int *ub,
int *id, int *cook_num);

/* Making a salad in random time between lb and ub */
void make_salad(int lb, int ub);   

int isTableEmpty(int *table){return table[0] == EMPTY;}

void getIntegrities(int *integrities, int *table)
{integrities[0] = table[0];table[0] = EMPTY;
integrities[1] = table[1];table[1] = EMPTY;}

int main ( int argc , char ** argv )
{
    int retval, id , err , lb, ub, cook_num;
    char timeStr[30], msg[100];
    Buffer *buffer;

    struct timeval t2, elapsed;


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

    buffer->cooks_pid[cook_num] = pid;

    int integrities[3];
    integrities[2] = cook_num;//Salad maker integrity

    /* Take integrities from chef(read) , write in log files */
    while(buffer->n_salands >= 0){

        /* Waiting for chef notification */
        sprintf(msg, "Waiting for integrities");

        //Writing in the logfiles
        fprintf(logfile, "%s\n", msg);
        fflush(logfile);

        sem_wait(&buffer->log);
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(global_log, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        fflush(global_log);

        sem_post(&buffer->log);

        sem_wait(&buffer->cooks[cook_num]);


        if(isTableEmpty(buffer->table))break; //All salads done while waiting


        /* Get integrities from table */
        sem_wait(&buffer->access_table);
        getIntegrities(integrities, buffer->table);
        sem_post(&buffer->access_table);

        fprintf(logfile,"salads remaining %d\n", buffer->n_salands);
        fflush(logfile);

        //Inform chef that integrities retrieved
        sem_post(&buffer->chef);

        //Writing in the logfiles
        sprintf(msg, "Get %s   %s", \
        str_integrities[integrities[0]], str_integrities[integrities[1]]);

        fprintf(logfile, "%s\n", msg);
        fflush(logfile);

        sem_wait(&buffer->log);
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(global_log, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        fflush(global_log);

        sem_post(&buffer->log);



        /* Start making salad */
        sprintf(msg, "Start making salad");

        //Writing in the logfiles
        fprintf(logfile, "%s\n", msg);
        fflush(logfile);

        sem_wait(&buffer->log);
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(global_log, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        fflush(global_log);

        sem_post(&buffer->log);

        make_salad(lb,ub);



        /* End making salad */
        sprintf(msg, "End making salad");

        //Writing in the logfiles
        fprintf(logfile, "%s\n", msg);
        fflush(logfile);

        sem_wait(&buffer->log);
        
        gettimeofday(&t2, NULL);
        time_elapsed(&elapsed, &t2, &buffer->t1);
        time_to_string(elapsed, timeStr);
        fprintf(global_log, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        fflush(global_log);

        /* decrement total salads */
        --buffer->n_salands;
        /* Iscrease salads done */
        ++buffer->saladsDone[cook_num];

        sem_post(&buffer->log);

    } 
    sem_post(&buffer->chef);
    
    
    /* Remove segment . */
    err = shmctl (id , IPC_RMID , 0) ;
    if (err == -1) perror (" Removal . ");
    else printf (" Removed . % d\n" , ( int )( err )) ;
    sem_destroy (&buffer->chef);

    shmdt((void *) 0);

    fclose(logfile);

    fclose(global_log);
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
    int worktime, timeRange = ub-lb;
    if(timeRange > 0)worktime = (rand() % (ub-lb)) + lb;
    else worktime = 0;
    usleep(worktime * 1000);
}