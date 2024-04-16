#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#define TOTAL_MEMORY 2048
#define MAX_QUEUE_SIZE 10
#define TOTAL_FRAMES 512 
//#include "paged.h"



typedef struct {
    char name[10];
    int serviceTime;
    int remainingTime;
    int startTime; 
    int finishTime; 
    int memoryRequirement;
    int memoryStart;
    int lastUsed;
    bool haspage;
    int pagesAllocated[TOTAL_FRAMES]; 
; 
     
} Process;

//prework and support functions/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Process* waitingQueue[MAX_QUEUE_SIZE];
int queueSize = 0; 


typedef struct {
    bool occupied;  // frame okay?
    char owner[10];  // which take that
    int page_number;
    int lastUsed;  // 
} Frame;

Frame frames[TOTAL_FRAMES];


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

//memory allocation and etc//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

bool allocateMemory(Process* process) {
    MemoryBlock* current = head;
    
    while (current != NULL) {
        if (strcmp(current->type, "hole") == 0 && current->length >= process->memoryRequirement) {
            //printf("Allocating memory for %s, memory required: %d blocks.\n", process->name, process->memoryRequirement);

            process->memoryStart = current->start;

            if (current->length == process->memoryRequirement) {
                strcpy(current->type, "process");
                //printf("Memory block exactly fits the requirement.\n");
            } else {
                MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));

                if (newBlock == NULL) {
                    //printf("Memory allocation for newBlock failed.\n");
                    return false;
                }

                newBlock->start = current->start + process->memoryRequirement;
                newBlock->length = current->length - process->memoryRequirement;
                strcpy(newBlock->type, "hole");
                newBlock->next = current->next;
                current->next = newBlock;
                current->length = process->memoryRequirement;
                strcpy(current->type, "process");
                //printf("Splitting memory block. New hole at %d, length %d.\n", newBlock->start, newBlock->length);
            }

            return true;
        }

        current = current->next;
        //printf("Next pointer of current node is: %p\n", (void*)current->next);
    }

    if(current==NULL){
        //printf("current now null");
    }
    //printf("Failed to allocate memory for process %s.\n", process->name);
    
    return false;
    //printf("already false");
}

void addToWaitingQueue(Process* process) {
//to check if the process already in
    for (int i = 0; i < queueSize; i++) {
        if (waitingQueue[i] == process) {
            //printf("Process %s is already in the waiting queue.\n", process->name);
            return;  
        }
    }


    if (queueSize < MAX_QUEUE_SIZE) {
        waitingQueue[queueSize] = process;
        queueSize++;

        //printf("Process %s added to waiting queue. in queue processes: %d\n", process->name, queueSize);
    } else {
        //printf("Waiting queue is full. Cannot add process %s.\n", process->name);
    }
}


void tryAllocateMemoryForWaitingProcesses() {
    int i = 0;

    while (i < queueSize) {
        Process* process = waitingQueue[i];

        if (allocateMemory(process)) {
            //printf("Memory allocated for %s from waiting queue.\n", process->name);
            
            for (int j = i; j < queueSize - 1; j++) {
                waitingQueue[j] = waitingQueue[j + 1];
            }

            queueSize--;

            //printf("Memory allocated for %s. Queue size now: %d\n", process->name, queueSize);
            //printf("remainpreocess:%d\n",processwaiting);
        } else {
            i++; 
        }
    }
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

////paged algorithm////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initFrames() {
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        frames[i].occupied = false;
        frames[i].owner[0] = '\0'; 
        frames[i].page_number = -1; 
    }
}
// void printPagesAllocated(Process* process) {
//     //printf("Pages allocated for process %s: [", process->name);
//     int first = 1;  // 用于格式化输出，避免在数组开始处打印逗号
//     for (int i = 0; i < TOTAL_FRAMES; i++) {
//         if (process->pagesAllocated[i] != -1) {  
//             if (!first) {
//                 printf(", ");
//             }
//             printf("%d", process->pagesAllocated[i]);
//             first = 0;
//         }
//     }
//     printf("]\n");
// }



bool allocatePages(Process *process) {
    int pagesNeeded = (process->memoryRequirement + 3) / 4; 
    int pagesdid = 0;


    for (int i = 0; i < TOTAL_FRAMES && pagesdid< pagesNeeded; i++) {
        if (!frames[i].occupied) {
            frames[i].occupied = true;
            strcpy(frames[i].owner, process->name);  
            frames[i].page_number = pagesdid;
            process->pagesAllocated[pagesdid] = i; 
            
            pagesdid++;
        }
    }

    // 
    if (pagesdid == pagesNeeded) {
        process->haspage = true;
         //printPagesAllocated(process);
        return true;
    } else {
        for (int i = 0; i < TOTAL_FRAMES; i++) {
            if (strcmp(frames[i].owner, process->name) == 0) { 
                frames[i].occupied = false;  
                frames[i].owner[0] = '\0';  
                frames[i].page_number = -1;  
            }
        }
        process->haspage = false; 
        //这里应该要直接消除之前分配的所有帧，但可能有更好的办法？
        return false;
    }
}


int* evictPage(Process processes[], int numProcesses, char* currentProcessName, int* count) {

    //printf("im doing evict\n");
    char lruProcessName[10];
    int minLastUsed = INT_MAX;
    int lruProcessIndex = -1;

    // 寻找LRU进程，并把当前进程占用的排除
    for (int i = 0; i < numProcesses; i++) {
        if (strcmp(processes[i].name, currentProcessName) != 0 && processes[i].lastUsed < minLastUsed) {
            minLastUsed = processes[i].lastUsed;
            strcpy(lruProcessName, processes[i].name);
            lruProcessIndex = i;
        }
    }

    int* evictedFrames = NULL;
    *count = 0;  // 使用指针参数输出帧数量

    // 驱逐
    if (lruProcessIndex != -1) {
        Process *lruProcess = &processes[lruProcessIndex];
        evictedFrames = malloc(TOTAL_FRAMES * sizeof(int));
        if (!evictedFrames) {
            printf("Memory allocation failed for evictedFrames.\n");
            return NULL;
        }
        
        for (int i = 0; i < TOTAL_FRAMES; i++) {
            if (strcmp(frames[i].owner, lruProcessName) == 0) {
                frames[i].occupied = false;
                frames[i].owner[0] = '\0';
                frames[i].page_number = -1;
                
                // 记录被驱逐的帧索引
                evictedFrames[(*count)++] = i;

                // 清除 pagesAllocated 数组中的对应条目
                for (int j = 0; j < TOTAL_FRAMES; j++) {
                    if (lruProcess->pagesAllocated[j] == i) {
                        lruProcess->pagesAllocated[j] = -1;
                        break;  
                    }
                }
            }
        }

        // 标记该进程没有页
        lruProcess->haspage = false;
    }

    if (*count > 0) {
        // 调整数组大小以匹配实际被驱逐的帧数
        evictedFrames = realloc(evictedFrames, (*count) * sizeof(int));
    } else {
        // 没有帧被驱逐，释放数组并设置为NULL
        free(evictedFrames);
        evictedFrames = NULL;
    }

    return evictedFrames;
}





double calculatePageUsage() {
    int occupiedFrames = 0;

    // 遍历所有帧，统计被占用的帧数
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (frames[i].occupied) {
            occupiedFrames++;
        }
    }

    
    double memUsage = (double)occupiedFrames / TOTAL_FRAMES * 100.0;
    return memUsage;
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
    initMemory();
    initFrames();

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
    processes[i].haspage = false;

    // 初始化 pagesAllocated 数组，将所有条目设置为 -1 表示未分配
    for (int j = 0; j < TOTAL_FRAMES; j++) {
        processes[i].pagesAllocated[j] = -1;
    }
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
                    if (processes[j].startTime <= currentTime && processes[j].remainingTime > 0) {
                        remainingProcesses++;
                        //printf("process remain=%d\n", remainingProcesses);
                    }
                }

           
                if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                    //printf("Current Time: %d, Current Process: %d\n", currentTime, currentProcess);

                    if (processes[currentProcess].memoryStart == -1) {
                        bool isMemoryAllocated = allocateMemory(&processes[currentProcess]);
                        //printf("Memory allocated %d\n", processes[currentProcess].memoryStart);
                    
                        if (!isMemoryAllocated) {
                            addToWaitingQueue(&processes[currentProcess]);
                            continue;
                        }
                    }

                    foundProcessToRun = true;
                    if (quantumCounter == 0 && (currentProcess != lastProcess || queueSize > 0)) {
                        while(currentTime % quantum != 0){
                            currentTime++;
                        }

                        memUsage = calculateMemoryUsage(processes, numProcesses);

                        printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n",
                                currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime,
                                (int)memUsage, processes[currentProcess].memoryStart);
                                
                        lastProcess = currentProcess;
                    }

                    if (++quantumCounter == quantum) {
                        
                        processes[currentProcess].remainingTime -= quantum;
                        //printf("remain--%d\n",processes[currentProcess].remainingTime);

                        currentTime += quantum;
                        quantumCounter = 0; 
                    


                 

                        if (processes[currentProcess].remainingTime <= 0) {
                            processes[currentProcess].finishTime = currentTime;
                            completedProcesses++;
                            freeMemory(&processes[currentProcess]);
                            //printf("free\n"); 
                            tryAllocateMemoryForWaitingProcesses();
                            
                            for (int k = 0; k < numProcesses; k++) {
                                if (processes[k].startTime == currentTime && k != currentProcess) {
                                    remainingProcesses++;
                                }
                            }
                            printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",
                                    currentTime, processes[currentProcess].name, remainingProcesses-1);
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
        //printf("free memory");
        if(processes[i].memoryStart != -1) {
            freeMemory(&processes[i]);
        }
    }
}



//paged mode////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
else if (strcmp(memoryStrategy, "paged") == 0){
    int currentProcess = -1;
    int lastProcess = -1;
    int quantumCounter = 0;
    double memUsage = 0;
    int freedCount = 0;
    //bool isdone=false;

    while(completedProcesses < numProcesses) {
        bool foundProcessToRun = false;

        for(int i = 0; i < numProcesses; i++) {
            currentProcess = (currentProcess + 1) % numProcesses;
            int remainingProcesses = 0;

            for(int j = 0; j < numProcesses; j++) {
                if (processes[j].startTime <= currentTime && processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {



                if (!processes[currentProcess].haspage) {
                    bool isMemoryAllocated = allocatePages(&processes[currentProcess]);

                    while (!isMemoryAllocated) {//这个Loop开始条件是：轮到某个进程了且分配失败
                        int* freedPages = evictPage(processes, numProcesses, processes[currentProcess].name, &freedCount);
                        //printf("Attempting to evict pages for process: %s,freepage=%n\n", processes[currentProcess].name, freedPages);
                        if (freedPages) {
                            printf("%d,EVICTED,evicted-frames=[", currentTime);
                            for (int i = 0; i < freedCount; i++) {
                                if (i > 0) printf(",");
                                printf("%d", freedPages[i]);
                            }
                            printf("]\n");
                            free(freedPages);
                        }
                        
                        

                        if (freedCount == 0) {
                            break;
                        }
                        
                        isMemoryAllocated = allocatePages(&processes[currentProcess]);
                    }
                    if (isMemoryAllocated) {
                        processes[currentProcess].lastUsed = currentTime;
                    }

                    if (!isMemoryAllocated) {
                        continue;
                    }
                }

                foundProcessToRun = true;
                if (quantumCounter == 0 && (currentProcess != lastProcess || queueSize > 0)) {
                    while(currentTime % quantum != 0){
                        currentTime++;
                    }

                    memUsage = calculatePageUsage();

                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[",
                        currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime,
                        (int)memUsage);

                    bool isFirst = true;

                    for (int i = 0; i < TOTAL_FRAMES; i++) {
                        if (strcmp(frames[i].owner, processes[currentProcess].name) == 0) {
                            if (!isFirst) printf(",");
                            printf("%d", i);
                            isFirst = false;
                        }
                    }
                    printf("]\n");

                    lastProcess = currentProcess;
                }

                if (++quantumCounter == quantum) {//这个loop的开始条件是：到整除时间片了，并且发现某个进程已经ok了
                    processes[currentProcess].remainingTime -= quantum;
                    currentTime += quantum;
                    quantumCounter = 0;



                    
                    
                    if (processes[currentProcess].remainingTime <= 0) {
                        
                        if (remainingProcesses == 1) { 
                            
                            for (int i = 0; i < numProcesses; i++) {
                                //printf("free memory");
                                if (processes[currentProcess].haspage) {
                                    int* freedPages = evictPage(processes, numProcesses, processes[i].name, &freedCount);
                                    //printf("freedcount%d\n",freedCount);
                                    if (freedPages) {
                                        printf("%d,EVICTED,evicted-frames=[", currentTime);
                                        for (int j = 0; j < freedCount; j++) {
                                            if (j > 0) printf(",");
                                            printf("%d", freedPages[j]);
                                        }
                                        printf("]\n");
                                        free(freedPages);
                                    }
                                }
                            }
                        }

                        // 更新完成时间和计数，打印完成信息
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



    //     for(int i = 0; i < numProcesses; i++) {
    //     //printf("free memory");
    //     if(processes[currentProcess].haspage) {
            
    //      int* freedPages2 = evictPage(processes, numProcesses, processes[i].name, &freedCount);
    //        //printf("freedcount%d\n",freedCount);

    //         if (freedPages2) {
                            
    //              printf("%d,EVICTED,evicted-frames=[", currentTime);
    //             for (int i = 0; i < freedCount; i++) {
    //                 if (i > 0) printf(",");
    //                 printf("%d", freedPages2[i]);
    //             }
    //             printf("]\n");
    //             free(freedPages2);
    //         }
    //     }
    // }
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