#include "fault_tolerance.h"
#include <stdio.h>
#include <unistd.h>

extern long long queue[QUEUE_SIZE];
extern int front,rear;
extern HashMap myMap;

void *periodicSave() {
    while (1) {
        sleep(1);
        saveQueueToFile();
        saveMapToFile(&myMap);
    }

    return NULL;
}

// Write queue data to a file
void saveQueueToFile() {
    FILE *file = fopen("queue_data.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "%d %d\n", front, rear);

    for (int i = front; i != rear; i = (i + 1) % QUEUE_SIZE) {
        fprintf(file, "%lld\n", queue[i]);
    }

    fclose(file);
}

// Load queue data from a file
void loadQueueFromFile() {
    FILE *file = fopen("queue_data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fscanf(file, "%d %d", &front, &rear);

    for (int i = front; i != rear; i = (i + 1) % QUEUE_SIZE) {
        fscanf(file, "%lld", &queue[i]);
    }

    fclose(file);
}


// Write hash map data to a file
void saveMapToFile(HashMap *map) {
    FILE *file = fopen("map_data.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < SIZE; i++) {
        struct KeyValuePair *current = map->array[i];
        while (current != NULL) {
            fprintf(file, "%ld %d\n", current->key, current->status);
            current = current->next;
        }
    }

    fclose(file);
}

// Load hash map data from a file
void loadMapFromFile(HashMap *map) {
    FILE *file = fopen("map_data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < SIZE; i++) {
        while (1) {
            long long key;
            int status;

            if (fscanf(file, "%lld %d", &key, &status) != 2) {
                break;
            }

            insert(&myMap, key, status);
        }
    }

    fclose(file);
}
