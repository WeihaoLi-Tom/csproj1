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
//memory allocation and etc.////////////////////////////////////////////////////////////////////////////////
bool memory[TOTAL_MEMORY] = {false};
Process* allocatedProcesses[5];
int allocatedProcessCount = 0;

typedef struct MemoryBlock {
    char type[6];  
    int start;
    int length;
    struct MemoryBlock* next;
} MemoryBlock;

MemoryBlock* head = NULL;

void initMemory() {
    
    head = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    strcpy(head->type, "hole");
    head->start = 0;
    head->length = TOTAL_MEMORY;
    head->next = NULL;
}


typedef struct AllocatedProcessNode {
    Process* process;
    struct AllocatedProcessNode* next;
} AllocatedProcessNode;
    
AllocatedProcessNode* allocatedHead = NULL; 



bool allocateMemory(Process* process) {
    initMemory();
    MemoryBlock* current = head;
    //MemoryBlock* prev = NULL;
     //printf("hi im here");
    while (current != NULL) {
        //printf("hi im in");
        if (strcmp(current->type, "hole") == 0 && current->length >= process->memoryRequirement) {
            
            printf("Allocating memory for %s, memory required: %d blocks.\n", process->name, process->memoryRequirement);

           
            process->memoryStart = current->start;

            
            if (current->length == process->memoryRequirement) {
                printf("hi if");
                strcpy(current->type, "process");
            } else {
                
                MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
                printf("cuttrnt %d\n", current->start);
                newBlock->start = current->start + process->memoryRequirement;
                newBlock->length = current->length - process->memoryRequirement;
                strcpy(newBlock->type, "hole");
                newBlock->next = NULL;
                current->next = newBlock;
                current->length = process->memoryRequirement;
                strcpy(current->type, "process");
                
            }
    printf("hi im out");
    
            return true;
        }
        //prev = current;
        current = current->next;
    }
    return false; 
}






void freeMemory(Process* process) {
    MemoryBlock* current = head;
    MemoryBlock* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->type, "process") == 0 && current->start == process->memoryStart) {
            //printf("Freeing memory for %s starting at %d\n", process->name, process->memoryStart);

        
            strcpy(current->type, "hole");

            // combine hole
            if (prev != NULL && strcmp(prev->type, "hole") == 0) {
                
                prev->length += current->length;
                prev->next = current->next;
                free(current);
                current = prev->next;
            } else {
                prev = current;
                current = current->next;
            }

           
            if (current != NULL && strcmp(current->type, "hole") == 0) {
                prev->length += current->length;
                prev->next = current->next;
                free(current);
            }

            return;
        }
        prev = current;
        current = current->next;
    }
}



double calculateMemoryUsage() {
    int totalMemoryUsed = 0;
    MemoryBlock* current = head;

    while (current != NULL) {
        if (strcmp(current->type, "process") == 0) {
            totalMemoryUsed += current->length;
        }
        current = current->next;
    }

    //printf("Debug: Total Memory Used: %d\n", totalMemoryUsed);
    double usage = ceil(((double)totalMemoryUsed / TOTAL_MEMORY) * 100);
    //printf("Memory Usage: %.2f%%\n", usage);
    return usage;
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
// for (int i = 0; i < numProcesses; i++) {
//     printf("Process %s, Memory Requirement: %d\n", processes[i].name, processes[i].memoryRequirement);
// }

    fclose(file);

for (int i = 0; i < numProcesses; i++) {
    processes[i].memoryStart = -1; 
}
    




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
                if(processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

           
            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                //printf("Current Time: %d, Current Process: %d\n", currentTime, currentProcess);

            
                if (processes[currentProcess].memoryStart == -1) {
                    
                   
                    bool isMemoryAllocated = allocateMemory(&processes[currentProcess]);
                    
                    //printf("Memory allocated %d\n", processes[currentProcess].memoryStart);
                    
                    if (!isMemoryAllocated) {
                        
                        continue;
                    }
                    memUsage = calculateMemoryUsage(processes, numProcesses); 
                    
                }

                foundProcessToRun = true;
                if (quantumCounter == 0 && currentProcess != lastProcess) {
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
                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d,\n",
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
