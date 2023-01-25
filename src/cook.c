#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "my_defines.h"
#include "utils.h"



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
    int id , err , lb, ub, cook_num;
    char timeStr[30], msg[100], file_msg[100];
    Buffer *buffer;


    /* Get input arguments */
    if(get_input_args(argc, argv, &lb, &ub, &id, &cook_num) == -1){
        printf("Usage: -t1 <lb> -t2 <lu> -s <shmid> -c <cook_num> \n");
        return -1;
    }


    /* Create a logfile for current salad maker
       and open the global logfile */
    FILE *logfile, *global_log;
    int pid = getpid();

    char filename[100];
    sprintf(filename, "salad_maker%d_%d.txt", cook_num, pid);
    printf("Creating logfile %s\n", filename);
    logfile = fopen(filename, "a");

    srand(time(NULL));


    /* Attach the segment . */
    buffer = (Buffer *)shmat(id ,( void *) 0, 0);
    if (buffer == ( void *) -1) { perror (" Attachment ."); exit (2) ;}
    global_log = fopen(buffer->logfile, "a");

    buffer->cooks_pid[cook_num] = pid;

    int integrities[3];
    integrities[2] = cook_num;//Salad maker integrity



    /*** Start making salads with chef's integrities ***/
    while(buffer->n_salands >= 0){



        /* Waiting for chef notification */

        //Write messages in logfiles and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Waiting for integrities");
        sprintf(file_msg, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        write_messages(msg, file_msg, logfile, global_log);

        sem_post(&buffer->log);
        //End of writing

        sem_wait(&buffer->cooks[cook_num]);

        //Check if all salads done while waiting
        if(isTableEmpty(buffer->table))break; 



        /* Get integrities from table */

        sem_wait(&buffer->access_table);
        getIntegrities(integrities, buffer->table);
        sem_post(&buffer->access_table);
        printf("remaining salads %d\n", buffer->n_salands);
        fflush(stdout);
 
        //Inform chef that integrities retrieved
        sem_post(&buffer->chef);

        //Write messages in logfiles and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Get %s   %s", \
        str_integrities[integrities[0]], str_integrities[integrities[1]]);
        sprintf(file_msg, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        write_messages(msg, file_msg, logfile, global_log);

        sem_post(&buffer->log);
        //End of writing



        /* Start making salad */

        //Write messages in logfiles and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Start making salad");
        sprintf(file_msg, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        write_messages(msg, file_msg, logfile, global_log);

        sem_post(&buffer->log);
        //End of writing


        make_salad(lb,ub);


        /* End making salad */

        //Write messages in logfiles and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "End making salad");
        sprintf(file_msg, "[%s] [%d] [Saladmaker%d] [%s]\n",timeStr, pid,\
        cook_num, msg);
        write_messages(msg, file_msg, logfile, global_log);
        //Update salads done
        --buffer->n_salands;
        ++buffer->saladsDone[cook_num];

        sem_post(&buffer->log);
        //End of writing

        printf("\n");
        fflush(stdout);  
    } 

    /*In case chef wating for salad maker take integrities, 
      while all salads done*/
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