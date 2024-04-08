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

//basic round robin///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void runRoundRobin(Process processes[], int numProcesses, int quantum, int *currentTime, int *completedProcesses) {
    int currentProcess = -1; 
    int lastProcess = -1;
    int quantumCounter = 0;

    while(*completedProcesses < numProcesses) {
        bool foundProcessToRun = false;
        for(int i = 0; i < numProcesses; i++) {
            currentProcess = (currentProcess + 1) % numProcesses;

            int remainingProcesses = 0;
            for(int j = 0; j < numProcesses; j++) {
                if(processes[j].startTime <= *currentTime && processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

            if (*currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                foundProcessToRun = true;
                
                if (quantumCounter == 0 && currentProcess != lastProcess) {
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d\n", *currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime);
                    lastProcess = currentProcess;
                }

                if (++quantumCounter == quantum) {
                    processes[currentProcess].remainingTime -= quantum;
                    *currentTime += quantum;

                    if (processes[currentProcess].remainingTime <= 0) {
                        processes[currentProcess].finishTime = *currentTime;
                        (*completedProcesses)++;
                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", *currentTime, processes[currentProcess].name, remainingProcesses - 1);
                    }

                    quantumCounter = 0;
                }
                break;
            }
        }

        if (!foundProcessToRun && quantumCounter == 0) {
            (*currentTime)++;
        }
    }
}

bool memory[TOTAL_MEMORY] = {false};
Process* allocatedProcesses[5];
int allocatedProcessCount = 0;

bool allocateMemory(Process* process) {
    printf("Allocating memory for %s, memory required: %d blocks.\n", process->name, process->memoryRequirement);
    int blocksNeeded = process->memoryRequirement;
    for (int i = 0; i <= TOTAL_MEMORY - blocksNeeded; i++) {
        bool spaceFound = true;
        for (int j = 0; j < blocksNeeded; j++) {
            if (memory[i + j]) {
                spaceFound = false;
                break;
            }
        }
        if (spaceFound) {
            for (int j = 0; j < blocksNeeded; j++) {
                memory[i + j] = true;
            }
            process->memoryStart = i; 
            allocatedProcesses[allocatedProcessCount++] = process;
            return true;
        }
    }
    return false;
}





void freeMemory(Process* process) {
    for (int i = process->memoryStart; i < process->memoryStart + process->memoryRequirement; i++) {
        memory[i] = false;
    }
    process->memoryStart = -1;
    for (int i = 0; i < allocatedProcessCount; i++) {
        if (allocatedProcesses[i] == process) {
            allocatedProcesses[i] = allocatedProcesses[--allocatedProcessCount];
            break;
        }
    }
}

double calculateMemoryUsage(Process processes[], int numProcesses) {
    int totalMemoryUsed = 0;
    
    for (int i = 0; i < allocatedProcessCount; i++) {
        totalMemoryUsed += allocatedProcesses[i]->memoryRequirement;
    }
    printf("Debug: Total Allocated Processes: %d\n", allocatedProcessCount);

    for (int i = 0; i < allocatedProcessCount; i++) {
        printf("Debug: Process %s, Memory Required: %d\n", allocatedProcesses[i]->name, allocatedProcesses[i]->memoryRequirement);
    }

    return ((double)totalMemoryUsed / TOTAL_MEMORY) * 100;
}





//Command build///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
    Process processes[10];
    int numProcesses = 0;
    char *filename = NULL;
    char *memoryStrategy = NULL; 
    int quantum = 0;
    int currentTime = 0;
    int completedProcesses = 0;

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
//for (int i = 0; i < numProcesses; i++) {
//    printf("Process %s, Memory Requirement: %d\n", processes[i].name, processes[i].memoryRequirement);
//}

    fclose(file);

    




//allocate algorithm//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Round Robin////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (strcmp(memoryStrategy, "infinite") == 0) {
        runRoundRobin(processes, numProcesses, quantum, &currentTime, &completedProcesses);
}

//First in first out////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
else if (strcmp(memoryStrategy, "first-fit") == 0) {
    int currentProcess = -1; 
    int lastProcess = -1;
    int quantumCounter = 0;
    double memUsage = 0;

    while(completedProcesses < numProcesses) {
        bool foundProcessToRun = false;
        for(int i = 0; i < numProcesses; i++) {
            currentProcess = (currentProcess + 1) % numProcesses;

           
            int remainingProcesses = 0;
            for(int j = 0; j < numProcesses; j++) {
                if(processes[j].startTime <= currentTime && processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                
                if (processes[currentProcess].memoryStart == 0) {
                    printf("Before allocation for %s - Allocated Process Count: %d\n", processes[currentProcess].name, allocatedProcessCount);
                    bool isMemoryAllocated = allocateMemory(&processes[currentProcess]);
                    printf("After allocation for %s - Allocated Process Count: %d\n", processes[currentProcess].name, allocatedProcessCount);
                    if (!isMemoryAllocated) {
                        printf("Failed to allocate memory for %s\n", processes[currentProcess].name);
                        continue; 
                    } 
                }

                foundProcessToRun = true;
                if (quantumCounter == 0 && currentProcess != lastProcess) {
                    memUsage = calculateMemoryUsage(processes, numProcesses);
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%.2f%%,allocated-at=%d\n",
                           currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime,
                           memUsage, processes[currentProcess].memoryStart);
                    lastProcess = currentProcess;
                }

                if (++quantumCounter == quantum) {
                    processes[currentProcess].remainingTime -= quantum;
                    currentTime += quantum;
                    quantumCounter = 0; 

                    if (processes[currentProcess].remainingTime <= 0) {
                        processes[currentProcess].finishTime = currentTime;
                        completedProcesses++;
                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",
                               currentTime, processes[currentProcess].name, remainingProcesses - 1);
                    }
                }
                break; 
            }
        }

        if (!foundProcessToRun && quantumCounter == 0) {
            currentTime++; 
        }
    }

    // 释放所有进程的内存
    for(int i = 0; i < numProcesses; i++) {
        if(processes[i].memoryStart != -1) {
            freeMemory(&processes[i]);
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
