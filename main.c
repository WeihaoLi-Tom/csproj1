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
    int nextQueueTime;
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
int* logPageAllocation(Process *process, int* count) {
    int *allocatedFrames = malloc(TOTAL_FRAMES * sizeof(int)); // 动态分配数组存储帧索引
    if (!allocatedFrames) {
        printf("Memory allocation failed for allocatedFrames.\n");
        return NULL;
    }

    *count = 0;  // 初始化计数器
    // printf("Pages allocated for process %s: [", process->name);
    // for (int i = 0; i < TOTAL_FRAMES; i++) {
    //     if (process->pagesAllocated[i] != -1) {
    //         allocatedFrames[(*count)++] = process->pagesAllocated[i]; // 将帧索引添加到数组
    //         printf("%s%d", (*count) > 1 ? ", " : "", process->pagesAllocated[i]);
    //     }
    // }
    // printf("]\n");

    if (*count > 0) {
        allocatedFrames = realloc(allocatedFrames, (*count) * sizeof(int)); // 调整数组大小以匹配实际分配的帧数量
    } else {
        free(allocatedFrames);
        allocatedFrames = NULL;
    }

    return allocatedFrames; // 返回分配帧索引的数组
}





bool allocatePages(Process *process) {
    int pagesNeeded = (process->memoryRequirement + 3) / 4;
    int pagesAllocated = 0;

    for (int i = 0; i < TOTAL_FRAMES && pagesAllocated < pagesNeeded; i++) {
        if (!frames[i].occupied) {
            frames[i].occupied = true;
            strcpy(frames[i].owner, process->name);
            frames[i].page_number = pagesAllocated;
            process->pagesAllocated[pagesAllocated] = i;
            
            pagesAllocated++;
        }
    }

    if (pagesAllocated == pagesNeeded) {
        process->haspage = true;
        int frameCount;
        int* allocatedFrames = logPageAllocation(process, &frameCount);  // 记录并获取分配的帧
        if (allocatedFrames) {
            // 可以使用 allocatedFrames 做进一步处理
            free(allocatedFrames);
        }
        return true;
    } else {
        for (int i = 0; i < TOTAL_FRAMES; i++) {
            if (strcmp(frames[i].owner, process->name) == 0) {
                frames[i].occupied = false;
                frames[i].owner[0] = '\0';
                frames[i].page_number = -1;
                process->pagesAllocated[i] = -1;
            }
        }
        process->haspage = false;
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


void evictDonePages(Process *process) {
    //printf("wtfffffffffffffffffffffffff\n");
    if (!process->haspage) return;  // 如果进程没有分配的页面，直接返回

    //printf("Evicting pages for process %s\n", process->name);
    for (int i = 0; i < TOTAL_FRAMES; i++) {
  // 如果此帧被该进程占用
            int frameIndex = process->pagesAllocated[i];
            frames[frameIndex].occupied = false;
            frames[frameIndex].owner[0] = '\0';
            frames[frameIndex].page_number = -1;
            process->pagesAllocated[i] = -1;  // 标记为未分配
        
    }
    process->haspage = false;  // 标记进程没有分配任何页面
}




double calculatePageUsage() {
    int occupiedFrames = 0;

    // 遍历所有帧，统计被占用的帧数
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (frames[i].occupied) {
            occupiedFrames++;
        }
    }

    
    double memUsage = ceil((double)occupiedFrames / TOTAL_FRAMES * 100.0);
    return memUsage;
}





void printPagesAllocated(int currentTime, Process *process) {
    printf("%d,EVICTED,evicted-frames=[", currentTime);
    int first = 1;  // 用于格式化输出，避免在数组开始处打印逗号
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (process->pagesAllocated[i] != -1) {  // 检查是否有分配的帧
            if (!first) {
                printf(",");
            }
            printf("%d", process->pagesAllocated[i]);
            first = 0; // 之后的打印需要逗号
        }
    }
    printf("]\n");
}


///virtual 用的辅助函数//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool allocatePagesvirtual(Process *process) {
    int pagesNeeded = (process->memoryRequirement + 3) / 4;  // 一个页面4个单位内存，向上取整
    int minPagesRequired = pagesNeeded < 4 ? pagesNeeded : 4;  // 如果所需页面小于4，就使用所需页面数，否则至少需要4
    int pagesAllocated = 0;

    // 首先尝试分配至少最小所需页面数
    for (int i = 0; i < TOTAL_FRAMES && pagesAllocated < minPagesRequired; i++) {
        if (!frames[i].occupied) {
            frames[i].occupied = true;
            strcpy(frames[i].owner, process->name);
            frames[i].page_number = pagesAllocated;
            process->pagesAllocated[pagesAllocated] = i;
            
            pagesAllocated++;
        }
    }

    // 如果已分配至少最小所需页面数，尝试分配更多页面直到满足全部需求
    if (pagesAllocated >= minPagesRequired) {
        for (int i = 0; i < TOTAL_FRAMES && pagesAllocated < pagesNeeded; i++) {
            if (!frames[i].occupied) {
                frames[i].occupied = true;
                strcpy(frames[i].owner, process->name);
                frames[i].page_number = pagesAllocated;
                process->pagesAllocated[pagesAllocated] = i;
                
                pagesAllocated++;
            }
        }
    }

    // 确认是否成功分配了足够的页面
    process->haspage = pagesAllocated >= minPagesRequired;
    if (process->haspage) {
        int frameCount;
        int* allocatedFrames = logPageAllocation(process, &frameCount);  // 记录并获取分配的帧
        if (allocatedFrames) {
            // 可以使用 allocatedFrames 做进一步处理
            free(allocatedFrames);
        }
    } else {
        // 如果未能分配到最小所需页面数，撤销所有已分配页面
        for (int i = 0; i < TOTAL_FRAMES; i++) {
            if (strcmp(frames[i].owner, process->name) == 0) {
                frames[i].occupied = false;
                frames[i].owner[0] = '\0';
                frames[i].page_number = -1;
                process->pagesAllocated[i] = -1;
            }
        }
    }
    return process->haspage;
}


void printProcessQueue(int* queue, Process* processes, int queueSize) {
    printf("Current Process Queue: [");
    for (int i = 0; i < queueSize; i++) {
        int pid = queue[i];
        printf("(%s, %d)", processes[pid].name, processes[pid].nextQueueTime);
        if (i < queueSize - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

void printProcessesLastUsed(Process* processes, int numProcesses) {
    printf("Processes last used times:\n");
    for (int i = 0; i < numProcesses; i++) {
        printf("Process %s last used at time %d\n", processes[i].name, processes[i].lastUsed);
    }
}




int* evictPagevirtual(Process processes[], int numProcesses, char* currentProcessName, int* count) {
    ///printf("virtual evicting!!!!!!!!!!!!!!!!!!!!!!!\n");
    int pagewehave=0;
    int* evictedFrames = malloc(TOTAL_FRAMES * sizeof(int));
    if (!evictedFrames) {
        printf("Memory allocation failed for evictedFrames.\n");
        return NULL;
    }

    *count = 0;
    int neededFrames = 4;
    int freeFrames = 0;

    // 首先计算当前空闲的帧数
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (!frames[i].occupied) {
            freeFrames++;
        }
    }

    // 如果空闲帧数少于4，则开始驱逐帧直到有至少4个空闲帧
    if (freeFrames < neededFrames) {
        int lruProcessIndex = -1;
        int lruTime = INT_MAX;

        // 寻找除当前进程之外的最小LRU进程
        for (int i = 0; i < numProcesses; i++) {
            if (processes[i].lastUsed < lruTime && processes[i].lastUsed >= 0 && strcmp(processes[i].name, currentProcessName) != 0) {
                lruTime = processes[i].lastUsed;
                lruProcessIndex = i;
            }
        }


                    for (int j = 0; j < TOTAL_FRAMES; j++) {
                        
                        if (strcmp(frames[j].owner, processes[lruProcessIndex].name) == 0) {

                            pagewehave++;

                        }
                    }
                    //printf("pagewehave=%d\n",pagewehave);


        if (lruProcessIndex != -1) {
            //printf("Evicting LRU process: %s\n", processes[lruProcessIndex].name);
            // 驱逐该进程的最多四个帧
            int evictions = 0;
            for (int i = 0; i < TOTAL_FRAMES && evictions < 4; i++) {
                if (frames[i].occupied && strcmp(frames[i].owner, processes[lruProcessIndex].name) == 0) {
                    frames[i].occupied = false;
                    frames[i].owner[0] = '\0';
                    frames[i].page_number = -1;
                    evictedFrames[(*count)++] = i;
                    processes[lruProcessIndex].pagesAllocated[i] = -1;
                    evictions++;
                }
            }
            // 检查该进程是否还有剩余的页
            // bool hasPages = false;
            // for (int i = 0; i < TOTAL_FRAMES; i++) {
            //     if (strcmp(frames[i].owner, processes[lruProcessIndex].name) == 0) {
            //         hasPages = true;
            //         break;
            //     }
            // }

            if (pagewehave==4) {
                processes[lruProcessIndex].haspage = false;
                //printf("All pages of %s have been evicted. Set haspage to false.\n", processes[lruProcessIndex].name);
            }
        }
    }

    if (*count > 0) {
        evictedFrames = realloc(evictedFrames, (*count) * sizeof(int)); // 调整数组大小
    } else {
        free(evictedFrames);
        evictedFrames = NULL;
    }

    return evictedFrames;
}


int* evictAllPages(Process processes[], int numProcesses, int* count) {
    int* evictedFrames = malloc(TOTAL_FRAMES * sizeof(int));
    if (!evictedFrames) {
        printf("Memory allocation failed for evictedFrames.\n");
        return NULL;
    }

    *count = 0; // 初始化驱逐帧计数器

    // 遍历所有帧，将它们标记为未占用，并记录被驱逐的帧索引
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (frames[i].occupied) {
            frames[i].occupied = false;
            frames[i].owner[0] = '\0';
            frames[i].page_number = -1;
            evictedFrames[(*count)++] = i;

            // 更新相关进程的 pagesAllocated
            for (int j = 0; j < numProcesses; j++) {
                if (processes[j].pagesAllocated[i] != -1) {
                    processes[j].pagesAllocated[i] = -1;
                    processes[j].haspage = false;
                }
            }
        }
    }

    if (*count > 0) {
        evictedFrames = realloc(evictedFrames, (*count) * sizeof(int)); // 调整数组大小以匹配实际被驱逐的帧数
    } else {
        free(evictedFrames);
        evictedFrames = NULL;
    }

    return evictedFrames;
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
    int currentProcess = 0;  
    int lastProcess = -1;
    int quantumCounter = 0;
    double memUsage = 0;
    int freedCount = 0;
    bool allPagesAllocated = false;


    // 使用动态数组模拟队列
    int *processQueue = malloc(numProcesses * sizeof(int));
    int queueSize = 0;
    for (int i = 0; i < numProcesses; i++) {
        processQueue[queueSize++] = i; // 初始化队列，所有进程入队
    }

    while(completedProcesses < numProcesses && queueSize > 0) {
        bool foundProcessToRun = false;

        // 循环尝试队列中的每个进程
        for(int i = 0; i < queueSize; i++) {
            currentProcess = processQueue[i];
            int remainingProcesses = 0;


            for(int j = 0; j < numProcesses; j++) {
                if (processes[j].startTime <= currentTime && processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {
                

                if (!processes[currentProcess].haspage) {
                    allPagesAllocated = allocatePages(&processes[currentProcess]);

                    //printPagesAllocated(&processes[currentProcess]);
                    processes[currentProcess].lastUsed=currentTime;

                    while (!allPagesAllocated) { // 如果分配失败
                        int* freedPages = evictPage(processes, numProcesses, processes[currentProcess].name, &freedCount);
                        if (freedPages) {
                            printf("%d,EVICTED,evicted-frames=[", currentTime);
                            for (int j = 0; j < freedCount; j++) {
                                if (j > 0) printf(",");
                                printf("%d", freedPages[j]);
                            }
                            printf("]\n");
                            free(freedPages);
                        }

                        if (freedCount == 0) { // 无页面可驱逐
                            break;
                        }

                        allPagesAllocated = allocatePages(&processes[currentProcess]);
                    }

                    if (!allPagesAllocated) {
                        continue; // 无法分配，尝试下一个进程
                    }
                }

                foundProcessToRun = true;
                if (quantumCounter == 0 && currentProcess != lastProcess) {
                    while(currentTime % quantum != 0){
                        currentTime++;
                    }

                    memUsage = calculatePageUsage();
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[",
                        currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime,
                        (int)memUsage);

                    // 输出分配的帧
                    bool isFirst = true;
                    for (int j = 0; j < TOTAL_FRAMES; j++) {
                        if (strcmp(frames[j].owner, processes[currentProcess].name) == 0) {
                            if (!isFirst) printf(",");
                            printf("%d", j);
                            isFirst = false;
                        }
                    }
                    printf("]\n");

                    lastProcess = currentProcess;
                }

                if (++quantumCounter == quantum || currentTime == 0) {
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
                            }else{

                        
                        //printf("Attempting to evict pages excpet : %s\n", processes[currentProcess-1].name);
                        //printf("but current process is: %s\n", processes[currentProcess].name);
                       // printf("free page number is:%d\n",freedCount);
                        //printf("current is:%d\n",currentProcess);
                        printPagesAllocated(currentTime,&processes[currentProcess]);
                        evictDonePages(&processes[currentProcess]);

                        }
                            
                        
                        processes[currentProcess].finishTime = currentTime;
                        completedProcesses++;

                        // 从队列中移除当前进程
                        for (int j = i; j < queueSize - 1; j++) {
                            processQueue[j] = processQueue[j + 1];
                        }
                        queueSize--;

                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",
                            currentTime, processes[currentProcess].name, queueSize);
                    } else {
                        // 将当前进程移至队尾
                        for (int j = i; j < queueSize - 1; j++) {
                            processQueue[j] = processQueue[j + 1];
                        }
                        processQueue[queueSize - 1] = currentProcess;
                    }
                }
                break;
            }
        }

        if (!foundProcessToRun && quantumCounter == 0) {
            currentTime++;
        }
    }

    free(processQueue);
}



//virtual///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
else if (strcmp(memoryStrategy, "virtual") == 0){
    int currentProcess = 0;  
    int lastProcess = -1;
    int quantumCounter = 0;
    double memUsage = 0;
    int freedCount = 0;
    bool allPagesAllocated = false;


    // 使用动态数组模拟队列
    int *processQueue = malloc(numProcesses * sizeof(int));
    int queueSize = 0;


    for (int i = 0; i < numProcesses; i++) {
        processQueue[queueSize++] = i; // 初始化队列，所有进程入队
        
    }
    for (int i = 0; i < numProcesses; i++) {
        processQueue[i] = i;
        processes[i].nextQueueTime = processes[i].startTime;
        processes[i].lastUsed=-1;
    }
    while(completedProcesses < numProcesses && queueSize > 0) {
        bool foundProcessToRun = false;

        // 循环尝试队列中的每个进程
        for(int i = 0; i < queueSize; i++) {
            
            currentProcess = processQueue[i];
            //printf("current is %s\n",processes[currentProcess].name);
            int remainingProcesses = 0;


            for(int j = 0; j < numProcesses; j++) {
                if (processes[j].startTime <= currentTime && processes[j].remainingTime > 0) {
                    remainingProcesses++;
                }
            }

            if (currentTime >= processes[currentProcess].startTime && processes[currentProcess].remainingTime > 0) {

                if (!processes[currentProcess].haspage) {
                    //printf("haspage?%d\n",processes[currentProcess].haspage);
                    allPagesAllocated = allocatePagesvirtual(&processes[currentProcess]);
                    //printf("allocate result,for=%d%s\n",allPagesAllocated,processes[currentProcess].name);

                    //printPagesAllocated(&processes[currentProcess]);
                    processes[currentProcess].lastUsed=currentTime;
                    //printProcessesLastUsed(processes, numProcesses);

                    while (!allPagesAllocated) { // 如果分配失败
                    //printf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk\n");
                        int* freedPages = evictPagevirtual(processes, numProcesses, processes[currentProcess].name, &freedCount);
                        if (freedPages) {
                            printf("%d,EVICTED,evicted-frames=[", currentTime);
                            for (int j = 0; j < freedCount; j++) {
                                if (j > 0) printf(",");
                                printf("%d", freedPages[j]);
                            }
                            printf("]\n");
                            free(freedPages);
                        }

                        if (freedCount == 0) { // 无页面可驱逐
                            break;
                        }

                        allPagesAllocated = allocatePagesvirtual(&processes[currentProcess]);
                    }

                    // if (!allPagesAllocated) {
                    //     continue; // 无法分配，尝试下一个进程
                    // }
                }

                foundProcessToRun = true;
                if (quantumCounter == 0 && currentProcess != lastProcess) {
                    while(currentTime % quantum != 0){
                        currentTime++;
                    }

                    memUsage = calculatePageUsage();
                    printf("%d,RUNNING,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[",
                        currentTime, processes[currentProcess].name, processes[currentProcess].remainingTime,
                        (int)memUsage);

                    // 输出分配的帧
                    bool isFirst = true;
                    for (int j = 0; j < TOTAL_FRAMES; j++) {
                        if (strcmp(frames[j].owner, processes[currentProcess].name) == 0) {
                            if (!isFirst) printf(",");
                            printf("%d", j);
                            isFirst = false;
                        }
                    }
                    printf("]\n");

                    lastProcess = currentProcess;
                }

                if (++quantumCounter == quantum || currentTime == 0) {
                    processes[currentProcess].remainingTime -= quantum;
                                        processes[currentProcess].lastUsed=currentTime;
                    //printProcessesLastUsed(processes, numProcesses);
                    currentTime += quantum;
                    quantumCounter = 0;


                    //processes[currentProcess].nextQueueTime =currentTime+1;
                    
                
                for (int i = 0; i < queueSize; i++) {
                    if (processes[processQueue[i]].nextQueueTime == currentTime) {
                        //printf("hi i find");
                        processes[currentProcess].nextQueueTime = currentTime + 1;
                        break;  
                    }else{
                        processes[currentProcess].nextQueueTime = currentTime;
                    }
                }


                    //printf("current time is %d\n",currentTime);
                    

                    if (processes[currentProcess].remainingTime <= 0) {

                        if (remainingProcesses == 1) { 
                            //printf("?????????????????????????????????\n");
                            for (int i = 0; i < numProcesses; i++) {
                                //printf("free memory");
                                if (processes[currentProcess].haspage) {
                                    int* freedPages = evictAllPages(processes, numProcesses, &freedCount);
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
                            }else{

                        
                        //printf("Attempting to evict pages excpet : %s\n", processes[currentProcess-1].name);
                        //printf("but current process is: %s\n", processes[currentProcess].name);
                       // printf("free page number is:%d\n",freedCount);
                        //printf("current is:%d\n",currentProcess);
                        printPagesAllocated(currentTime,&processes[currentProcess]);
                        evictDonePages(&processes[currentProcess]);

                        }
                            
                        
                        processes[currentProcess].finishTime = currentTime;
                        completedProcesses++;

                        // 从队列中移除当前进程
                        for (int j = i; j < queueSize - 1; j++) {
                            processQueue[j] = processQueue[j + 1];
                        }
                        queueSize--;

                        printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n",
                            currentTime, processes[currentProcess].name, queueSize);
                    } else {
        for (int j = i; j < queueSize - 1; j++) {
            processQueue[j] = processQueue[j + 1];
        }
        queueSize--;

        // 使用插入排序将当前进程插入回队列
int newPos;
for (newPos = 0; newPos < queueSize; newPos++) {
    if (processes[processQueue[newPos]].nextQueueTime > processes[currentProcess].nextQueueTime ||
        (processes[processQueue[newPos]].nextQueueTime == processes[currentProcess].nextQueueTime &&
         processes[processQueue[newPos]].lastUsed > processes[currentProcess].lastUsed)) {
        break;
    }
}

// 移动现有元素以为新元素腾出空间
for (int j = queueSize; j > newPos; j--) {
    processQueue[j] = processQueue[j - 1];
}

// 插入当前进程到计算出的位置
processQueue[newPos] = currentProcess;
queueSize++;

        //printProcessQueue(processQueue, processes, queueSize);
                    }
                }
                break;
            }
        }

        if (!foundProcessToRun && quantumCounter == 0) {
            currentTime++;
        }
    }

    free(processQueue);
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