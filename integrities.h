#define TABLE_INTEGRITIES 2
#define N_COOKS 3

struct Buffer{
    int table[2];
    sem_t cooks[3];
    sem_t chef;
};

enum Integrities{
    tomato, green_pepper, small_onion
};

char *str_integrities[] = { "tomato", "green_pepper", "small_onion" };
