
#define TABLE_INTEGRITIES 2
#define N_SALAD_MAKERS 3
#define SEGMENTSIZE sizeof(struct Buffer)
#define SEGMENTPERM 0666

#define EMPTY -1

typedef struct Buffer{
    int table[2];       //Two integrities in table
    sem_t cooks[3];   
    sem_t chef;
    sem_t log;          //Writing in logfile
    sem_t access_table; //Take/place integrities from table (write) 
    struct timeval t1;
    int n_salands;
    char logfile[30];
}Buffer;

enum Integrities{
    tomato, green_pepper, small_onion
};

char *str_integrities[] = { "tomato", "green_pepper", "small_onion" };
