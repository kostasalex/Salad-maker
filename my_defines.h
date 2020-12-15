
#define TABLE_INTEGRITIES 2
#define N_COOKS 3
#define SEGMENTSIZE sizeof(struct Buffer)
#define SEGMENTPERM 0666

typedef struct Buffer{
    int table[2];
    sem_t cooks[3];
    sem_t chef;
    sem_t test;
    int n_salands;
}Buffer;

enum Integrities{
    tomato, green_pepper, small_onion
};

char *str_integrities[] = { "tomato", "green_pepper", "small_onion" };
