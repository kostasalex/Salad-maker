#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include "my_defines.h"
#include "utils.h"



int  get_input_args(int argc, char *argv[], int *numOfSalads, int *mantime);

/* Returns 2 integrities with different combination from previous */
void get_integrities(int *integrities);

/* Finds the proper saladmaker */
int get_cook_num(int *integrities);

/* Put integrities on the table*/
void place_integrities(int *table, int *integrities);


void rest(int mantime){ usleep(1000 * mantime); }

/* Inform all salad makers that job is done */
void post_saladMakers(sem_t *saladMakers);



int main(int argc, char *argv[]){

    srand(time(NULL));
    
    char timeStr[30], msg[100], file_msg[100];
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
    printf("Creating a global logfile %s\n", filename);
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
    retval = sem_init(&buffer->chef,1 ,0); 
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit(3) ;}

    retval = sem_init(&buffer->log,1 ,1);
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit(3) ;}

    retval = sem_init (&buffer->access_table,1 ,1) ;
    if ( retval != 0) {
    perror ("Couldn ’t initialize ."); exit(3) ;}

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        retval = sem_init (&buffer->cooks[i],1 ,0) ;
        if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit(3) ;}
    }


    /*** Start giving integrities to saladmakers ***/
    int cook_num, integrities[2];
    while(buffer->n_salands >= 0){

        

        /* Select two integrities */

        get_integrities(integrities);

        //Write messages in global logfile and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Selecting integrities %s  %s"\
        ,str_integrities[integrities[0]], str_integrities[integrities[1]]);
        sprintf(file_msg, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        write_messages(msg, file_msg, logfile, NULL);

        sem_post(&buffer->log);
        //End of writing


        //Find the proper salad maker, for the selected integrities
        cook_num = get_cook_num(integrities);

        //Place integrities in table
        sem_wait(&buffer->access_table);
        place_integrities(buffer->table, integrities);
        sem_post(&buffer->access_table);




        /*  Notify the saladmaker */

        //Write messages in global logfile and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Notify saladmaker %d", cook_num);
        sprintf(file_msg, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        write_messages(msg, file_msg, logfile, NULL);

        sem_post(&buffer->log);
        //End of writing
        
        //Before notify saladmaker check if job is done
        if(buffer->n_salands <= 0)break;
        sem_post(&buffer->cooks[cook_num]);

        //wait salad maker to take the integrities

        if(buffer->n_salands <= 0)break;
        sem_wait(&buffer->chef);
        printf("remaining salads %d\n", buffer->n_salands);
        fflush(stdout);



        /* Man time for resting  */

        //Write messages in global logfile and stdout
        sem_wait(&buffer->log);  

        time_to_string(timeStr);
        sprintf(msg, "Man time for resting");
        sprintf(file_msg, "[%s] [%d] [chef] [%s]\n",timeStr, pid, msg);
        write_messages(msg, file_msg, logfile, NULL);

        sem_post(&buffer->log);
        //End of writing
        
        if(buffer->n_salands <= 0)break;
        rest(mantime);
        printf("\n");
        fflush(stdout);

    } 

    /* Clean table */
    buffer->table[0] = EMPTY;
    buffer->table[1] = EMPTY;


    printf("Inform salad makers that job is done \n");
    post_saladMakers(buffer->cooks);



    /***     Stdout of chef     ***/

    /* Print the number of salads made */
    printf("\nTotal #salads: %d\n", numOfSalads - buffer->n_salands);

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        printf("salads of salad_maker%d [%d]: %d\n", \
        i, buffer->cooks_pid[i], buffer->saladsDone[i]);
    }


    /* Read file and save all needed logs in records */
    fclose(logfile);
    fopen(filename, "r");
    char ch;

    //Count the number of records
    int lines = 0;
    while((ch = fgetc(logfile)) != EOF)
        if(ch == '\n')lines++;

    //Save records
    rewind(logfile);
    Record records[lines];
    get_records(logfile, records, lines, getpid());



    int working_saladmaker[N_SALAD_MAKERS], has_concurent = 0, start = -1,
    prev_concurent, concurent;
    for(int i = 0; i < N_SALAD_MAKERS; i++)working_saladmaker[i] = 0;

    /* Print all time intervals and the salad makers that working concurrently */
    printf("\nTime intervals: (in inscreasing order)\n");
    for(int r = 0; r < lines; r++){

        prev_concurent = count_concurent(working_saladmaker, N_SALAD_MAKERS);
        
        /* Start working */
        if(!strcmp(records[r].msg_part, "Get")){           
            working_saladmaker[records[r].cook_num]++;
            if((concurent = 
            count_concurent(working_saladmaker, N_SALAD_MAKERS)) > prev_concurent){
                
                /* Start of time interval with 3 concurent processes */
                if(concurent == 3){
                    if(start != -1){//Print the previous time interval with 2 processes
                        printf("[%s - %s]", records[start].time, records[r-1].time );
                       
                        //Print saladmakers that worked concurrently 
                        for(int j = 0; j < N_SALAD_MAKERS; j++)
                        {
                            if(working_saladmaker[j] && j != records[r].cook_num)
                                printf(" saladmaker%d ", j);
                        }
                        printf("\n");
                    }
                }
                /* Start of time interval with 2 or more processes */
                if(concurent >= 2) start = r;
            }
        }

        /* End working  */
        else if(!strcmp(records[r].msg_part, "End") || r == lines-1){
            
            /* End of for time interval with 2 or 3 concurrent processes */
            if((concurent = 
            count_concurent(working_saladmaker,N_SALAD_MAKERS)) >= 2 && start != -1){
                printf("[%s - %s]", records[start].time, records[r].time );
                
                //Print saladmakers that worked concurrently 
                for(int j = 0; j < N_SALAD_MAKERS; j++)
                    if(working_saladmaker[j])printf(" saladmaker%d ", j);

                printf("\n");
                if(concurent == 2)
                    start = -1;//Flag that interval beeing printed
                else if(r+1 < lines) start = r+1;//Set the next time interval
            }

            working_saladmaker[records[r].cook_num] = 0;
        }

        if(count_concurent(working_saladmaker, N_SALAD_MAKERS) >= 2)
            has_concurent = 1;
    }

    if(!has_concurent)printf("No concured processes found\n");



    /* Destroy semaphores */
    retval = sem_destroy(&buffer->chef) ;
    if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}

    retval = sem_destroy(&buffer->log) ;
    if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}

    retval = sem_destroy(&buffer->access_table) ;
    if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}

    for(int i = 0; i < N_SALAD_MAKERS; i++){
        retval = sem_destroy(&buffer->cooks[i]) ;
        if ( retval != 0) {
        perror ("Couldn ’t initialize ."); exit (3) ;}
    }

    shmdt((void *) 0);

    fclose(logfile);
    return 0;

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



void get_integrities(int *integrities){

    int integrity;

    integrities[0] = rand() % 3;

    while((integrity = rand() % 3) ==  integrities[0] || integrity == integrities[1]);

    integrities[1] = integrity;


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



void place_integrities(int *table, int *integrities)
{
table[0] = integrities[0]; 
table[1] = integrities[1];
}



void post_saladMakers(sem_t *saladMakers)
{   
    for(int i = 0; i < N_SALAD_MAKERS; i++)
        sem_post(&saladMakers[i]);
}