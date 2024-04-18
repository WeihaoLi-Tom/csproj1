#include <limits.h>

int* evictPagevirtual(Process processes[], int numProcesses, char* currentProcessName, int* count, int currentTime) {
    int* evictedFrames = malloc(TOTAL_FRAMES * sizeof(int));
    if (!evictedFrames) {
        printf("Memory allocation failed for evictedFrames.\n");
        return NULL;
    }

    *count = 0;
    int neededFrames = 4;
    int freeFrames = 0;

    // Count the number of free frames
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (!frames[i].occupied) {
            freeFrames++;
        }
    }

    // If there are not enough free frames, evict pages based on LRU
    if (freeFrames < neededFrames) {
        // Initialize the minimum LRU time to the maximum possible value
        int minLRU = INT_MAX;
        // Find the minimum LRU time among the processes
        for (int p = 0; p < numProcesses; p++) {
            if (strcmp(processes[p].name, currentProcessName) != 0 && processes[p].lastUsedTime < minLRU) {
                minLRU = processes[p].lastUsedTime;
            }
        }

        // Evict pages based on LRU
        for (int p = 0; p < numProcesses && freeFrames < neededFrames; p++) {
            if (strcmp(processes[p].name, currentProcessName) != 0 && processes[p].lastUsedTime == minLRU) {
                // Evict pages belonging to this process
                for (int i = 0; i < TOTAL_FRAMES && freeFrames < neededFrames; i++) {
                    if (frames[i].occupied && strcmp(frames[i].owner, processes[p].name) == 0) {
                        frames[i].occupied = false;
                        frames[i].owner[0] = '\0';
                        frames[i].page_number = -1;
                        evictedFrames[(*count)++] = i;
                        freeFrames++;
                    }
                }
            }
        }
    }

    // Reallocate memory for evictedFrames if necessary
    if (*count > 0) {
        evictedFrames = realloc(evictedFrames, (*count) * sizeof(int));
    } else {
        free(evictedFrames);
        evictedFrames = NULL;
    }

    return evictedFrames;
}
