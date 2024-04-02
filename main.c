#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#define TOTAL_MEMORY 2048

typedef struct {
    char name[10];
    int serviceTime;
    int remainingTime;
    int startTime; 
    int finishTime; 
    int memoryRequirement;
    int memoryStart; 
    
} Process;
//prework and support functions/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool memory[TOTAL_MEMORY] = {false}; 

int allocateFirstFit(int size) {
    for (int start = 0; start <= TOTAL_MEMORY - size; start++) {
        bool isSpaceEnough = true;
        for (int i = start; i < start + size; i++) {
            if (memory[i]) {
                isSpaceEnough = false;
                break;
            }
        }
        if (isSpaceEnough) {
            for (int i = start; i < start + size; i++) {
                memory[i] = true;
            }
            return start;
        }
    }
    return -1;
}


int calculateMemoryUsage() {
    int usedBlocks = 0;
    for (int i = 0; i < TOTAL_MEMORY; i++) {
        if (memory[i]) usedBlocks++;
    }
    
    return (int)ceil((double)usedBlocks * 100 / TOTAL_MEMORY);
}


void releaseMemory(int start, int size) {
    for (int i = start; i < start + size; i++) {
        memory[i] = false;
    }
}


//Command build///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    Process processes[10];
    int numProcesses = 0;
    char *filename = NULL;
    char *memoryStrategy = NULL; 
    int quantum = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            memoryStrategy = argv[++i]; 
        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            quantum = atoi(argv[++i]);
        }
    }

// scan the file ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (filename == NULL || quantum == 0) {
        fprintf(stderr, "Usage: %s -f <filename> -q <quantum>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return 1;
    }



while(fscanf(file, "%d %s %d %d", &processes[numProcesses].startTime, processes[numProcesses].name, &processes[numProcesses].serviceTime, &processes[numProcesses].memoryRequirement) != EOF) {
    processes[numProcesses].remainingTime = processes[numProcesses].serviceTime;
    numProcesses++;
}

    fclose(file);

    int currentTime = 0;
    int completedProcesses = 0;

 
  



//allocate algorithm//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Round Robin////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if (strcmp(memoryStrategy, "infinite") == 0) {
    int currentProcess = -1; 
    int lastProcess = -1;
    int quantumCounter = 0; 

    while(completedProcesses < numProcesses) {
        bool foundProcessToRun = false;
        for(int i = 0; i < numProcesses; i++) {
            currentProcess = (currentProcess + 1) % numProcesses;
            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                foundProcessToRun = true;
                // If the current process is different from the last one, print the RUNNING message
                if (quantumCounter == 0 && currentProcess != lastProcess ) { // Only print at the start of a quantum
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime);
                    lastProcess = currentProcess; 
                }

                // Simulate the process running for the quantum duration
                if (++quantumCounter == quantum) {
                    processes[currentProcess].remainingTime -= quantum;
                    if (processes[currentProcess].remainingTime <= 0) {
                        processes[currentProcess].finishTime = currentTime + quantum; // Adjust for the quantum duration
                        completedProcesses++;
                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", currentTime + quantum, processes[currentProcess].name, numProcesses - completedProcesses);
                    }
                    // Reset for the next process or quantum
                    quantumCounter = 0;
                    currentTime += quantum; // Update currentTime only after a whole quantum has passed
                }
                break; // Exit the loop to simulate the end of quantum or process completion
            }
        }

        if (!foundProcessToRun && quantumCounter == 0) {
            currentTime++; // If no process is running in this quantum, increment currentTime
        }
    }
}








//First in first out////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
else if (strcmp(memoryStrategy, "first-fit") == 0) {
    bool allProcessesBlocked = false; 

    while(completedProcesses < numProcesses && !allProcessesBlocked) {
        allProcessesBlocked = true; 

        for(int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];

            if(p->remainingTime > 0) {
                if (p->memoryStart == -1) { 
                    int memStart = allocateFirstFit(p->memoryRequirement);
                    if (memStart != -1) {
                        p->memoryStart = memStart;
                        allProcessesBlocked = false; 
                    } else {
                        continue; 
                    }
                }

                int memUsage = calculateMemoryUsage();
                printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", currentTime, processes[i].name, processes[i].remainingTime, memUsage, processes[i].memoryStart);
                int timeToRun = (p->remainingTime < quantum) ? p->remainingTime : quantum;
                p->remainingTime -= timeToRun;
                currentTime += timeToRun;

                if (p->remainingTime <= 0) {
                    printf("%d FINISHED process-name=%s\n", currentTime, p->name);
                    completedProcesses++;
                    releaseMemory(p->memoryStart, p->memoryRequirement); 
                    p->memoryStart = -1; 
                }

                if (completedProcesses < numProcesses) {
                    allProcessesBlocked = false;
                }
            }
        }
    }
}










//output result////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    double totalTurnaroundTime = 0;
    double totalOverhead=0;
    double maxOverhead=0.00;
    for(int i = 0; i < numProcesses; i++) {
        totalTurnaroundTime += processes[i].finishTime - processes[i].startTime;
        int turnaroundTime = processes[i].finishTime - processes[i].startTime;
        double timeOverhead = (double)turnaroundTime / processes[i].serviceTime;
        totalOverhead += timeOverhead;

        if (timeOverhead > maxOverhead) {
            maxOverhead = timeOverhead;
            
        }

        

        
    }
    double averageTurnaroundTime = totalTurnaroundTime / numProcesses;
    
    

    double averageTimeOverhead =totalOverhead / completedProcesses ; 

    printf("Turnaround time %.0f\n", ceil(averageTurnaroundTime));
    printf("Time overhead %.2f %.2f\n", maxOverhead, averageTimeOverhead);
    printf("Makespan %d\n", currentTime);

//     for(int i = 0; i < numProcesses; i++) {
//     printf("Process %s started at %d and finished at %d\n", 
//            processes[i].name, 
//            processes[i].startTime, 
//            processes[i].finishTime);
// }

    return 0;
}
