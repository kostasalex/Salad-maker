
#define TABLE_INTEGRITIES 2
#define N_SALAD_MAKERS 3
#define SEGMENTSIZE sizeof(struct Buffer)
#define SEGMENTPERM 0666

#define EMPTY -1

typedef struct Buffer{
    int table[TABLE_INTEGRITIES];    //Two integrities in table
    int cooks_pid[N_SALAD_MAKERS];
    int saladsDone[N_SALAD_MAKERS];
    sem_t cooks[N_SALAD_MAKERS];   
    sem_t chef;
    sem_t log;                      //Access logfile and salad counter
    sem_t access_table;             //Take/place integrities from table (write) 
    struct timeval t1;
    int n_salands;
    char logfile[30];
}Buffer;

enum Integrities{
    tomato, green_pepper, small_onion
};

char *str_integrities[] = { "tomato", "green_pepper", "small_onion" };
