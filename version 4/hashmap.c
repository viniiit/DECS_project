#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"


// Hash function to convert a string key into an index
int hash(long int key) {
    return key % SIZE;
}

int update_status(HashMap *map, long key,int status)
{
    int index = hash(key);
     struct KeyValuePair *current = map->array[index];

    // Traverse the linked list at this index to find the key
    while (current != NULL) {
        if (current->key==key) {
            // Key found, update status
            current->status=status;
            return 0;
        }
        current = current->next;
    }
    return 1;
}

// Function to create a new key-value pair
struct KeyValuePair *createKeyValuePair(long int key,int status) {
    struct KeyValuePair *newPair = (struct KeyValuePair *)malloc(sizeof(struct KeyValuePair));
    if (newPair == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    newPair->key = key;
    newPair->status=status;
    newPair->next = NULL;

    return newPair;
}

// Function to insert a key-value pair into the hash map
void insert(HashMap *map, long key,int status) {
    int index = hash(key);

    // Create a new key-value pair
    struct KeyValuePair *newPair = createKeyValuePair(key,status);

    // Check if there is already a key-value pair at this index
    if (map->array[index] == NULL) {
        // If the slot is empty, insert the new pair
        map->array[index] = newPair;
    } else {
        // If there is a collision, add the new pair to the linked list at this index
        newPair->next = map->array[index];
        map->array[index] = newPair;
    }
}


int know_status(HashMap *map, long int key) {
    int index = hash(key);
    struct KeyValuePair *current = map->array[index];

    // Traverse the linked list at this index to find the key
    while (current != NULL) {
        if (current->key==key) {
            // Key found, return the corresponding value
            return current->status;
        }
        current = current->next;
    }

    // Key not found
    return -1;
}
