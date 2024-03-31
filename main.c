#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[10];
    int arrivalTime; 
    int serviceTime;
    int remainingTime;
    int finishTime;  
} Process;

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


    if (filename == NULL || memoryStrategy == NULL || quantum == 0) {
        fprintf(stderr, "Usage: %s -f <filename> -m <memory-strategy> -q <quantum>\n", argv[0]);
        return 1;
    }

  
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return 1;
    }

    while(fscanf(file, "%*d %s %d %*d", processes[numProcesses].name, &processes[numProcesses].serviceTime) != EOF) {
        processes[numProcesses].remainingTime = processes[numProcesses].serviceTime;
        
        numProcesses++;
    }
    fclose(file);

    int currentTime = 0;
    int completedProcesses = 0;

    //for recording

    int currentTime = 0;
    int completedProcesses = 0;
    int totalServiceTime = 0;
    double maxTimeOverhead = 0.0;


while(completedProcesses < numProcesses) {
    for(int i = 0; i < numProcesses; i++) {
        if(processes[i].remainingTime > 0) {
            
            printf("%d RUNNING process-name=%s remaining-time=%d\n", currentTime, processes[i].name, processes[i].remainingTime);
            
            
            int timeToRun = processes[i].remainingTime < quantum ? processes[i].remainingTime : quantum;
            
            
            processes[i].remainingTime -= timeToRun;
            
            
            currentTime += timeToRun;
            
            
            if(processes[i].remainingTime == 0) {
                completedProcesses++;
                int remainingProcesses = numProcesses - completedProcesses;
                printf("%d FINISHED process-name=%s, proc-remaining=%d\n", currentTime, processes[i].name, remainingProcesses);

            }
        }
    }
}
    for (int i = 0; i < numProcesses; i++) {
        int turnaroundTime = processes[i].finishTime - processes[i].arrivalTime;
        totalServiceTime += processes[i].serviceTime;
        double timeOverhead = (double)turnaroundTime / processes[i].serviceTime;
        maxTimeOverhead = fmax(maxTimeOverhead, timeOverhead);
    }

    double averageTurnaroundTime = (double)totalServiceTime / numProcesses;
    double averageTimeOverhead = (double)totalServiceTime / totalServiceTime;

    // 打印性能统计信息
    printf("Turnaround time %d\n", (int)ceil(averageTurnaroundTime));
    printf("Time overhead %.2f %.2f\n", maxTimeOverhead, averageTimeOverhead);
    printf("Makespan %d\n", currentTime);


    return 0;
}
