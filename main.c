#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    char name[10];
    int serviceTime;
    int remainingTime;
    int startTime; 
    int finishTime; 
} Process;

int main(int argc, char *argv[]) {
    Process processes[2];
    int numProcesses = 0;
    char *filename = NULL;
    int quantum = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {

        } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
            quantum = atoi(argv[++i]);
        }
    }

    if (filename == NULL || quantum == 0) {
        fprintf(stderr, "Usage: %s -f <filename> -q <quantum>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Unable to open file");
        return 1;
    }

while(fscanf(file, "%d %s %d %*d", &processes[numProcesses].startTime, processes[numProcesses].name, &processes[numProcesses].serviceTime) != EOF) {
    processes[numProcesses].remainingTime = processes[numProcesses].serviceTime;
    numProcesses++;
}
    fclose(file);

    int currentTime = 0;
    int completedProcesses = 0;

 
    double maxTimeOverhead = 0.0;

while(completedProcesses < numProcesses) {
    for(int i = 0; i < numProcesses; i++) {
        if(processes[i].remainingTime > 0) {
            if(processes[i].startTime == -1) processes[i].startTime = currentTime;

           
            int timeToRun = quantum;
            printf("%d RUNNING process-name=%s remaining-time=%d\n", currentTime, processes[i].name, processes[i].remainingTime);
            
           
            processes[i].remainingTime -= timeToRun;

            
            currentTime += timeToRun;

            
            if(processes[i].remainingTime <= 0) {
                processes[i].finishTime = currentTime; 
                completedProcesses++;
                int remainingProcesses = numProcesses - completedProcesses;
                printf("%d FINISHED process-name=%s, proc-remaining=%d\n", currentTime, processes[i].name, remainingProcesses);

                
                int turnaroundTime = processes[i].finishTime - processes[i].startTime;
                double timeOverhead = (double)turnaroundTime / processes[i].serviceTime;
                if(timeOverhead > maxTimeOverhead) maxTimeOverhead = timeOverhead;
            }
        }
    }
}

 
    double totalTurnaroundTime = 0;
    for(int i = 0; i < numProcesses; i++) {
        totalTurnaroundTime += processes[i].finishTime - processes[i].startTime;
        
    }
    double averageTurnaroundTime = totalTurnaroundTime / numProcesses;
    
    

    double averageTimeOverhead = maxTimeOverhead; 

    printf("Turnaround time %.0f\n", ceil(averageTurnaroundTime));
    printf("Time overhead %.2f %.2f\n", maxTimeOverhead, averageTimeOverhead);
    printf("Makespan %d\n", currentTime);

    for(int i = 0; i < numProcesses; i++) {
    printf("Process %s started at %d and finished at %d\n", 
           processes[i].name, 
           processes[i].startTime, 
           processes[i].finishTime);
}

    return 0;
}
