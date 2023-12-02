#define SIZE 10

struct KeyValuePair {
    long int key;
    int status;
    struct KeyValuePair *next;
};

typedef struct{
    struct KeyValuePair *array[SIZE];
}HashMap;

void insert(HashMap *, long ,int);
struct KeyValuePair *createKeyValuePair(long int,int);
int know_status(HashMap *, long int);
int hash(long int);
int update_status(HashMap *, long int,int);