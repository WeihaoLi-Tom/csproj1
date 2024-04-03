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

<<<<<<< Updated upstream
=======
bool memory[TOTAL_MEMORY] = {false}; 



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


//Command build///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
>>>>>>> Stashed changes
int main(int argc, char *argv[]) {
    Process processes[2];
    int numProcesses = 0;
    char *filename = NULL;
    int quantum = 0;
    int currentTime = 0;
    int completedProcesses = 0;

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

<<<<<<< Updated upstream
    int currentTime = 0;
    int completedProcesses = 0;
=======

    
>>>>>>> Stashed changes

 
    double maxTimeOverhead = 0.0;

while(completedProcesses < numProcesses) {
    for(int i = 0; i < numProcesses; i++) {
        if(processes[i].remainingTime > 0) {
            if(processes[i].startTime == -1) processes[i].startTime = currentTime;

<<<<<<< Updated upstream
           
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
=======

//allocate algorithm//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Round Robin////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (strcmp(memoryStrategy, "infinite") == 0) {
        runRoundRobin(processes, numProcesses, quantum, &currentTime, &completedProcesses);
    }








//First in first out////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
else if (strcmp(memoryStrategy, "first-fit") == 0) {

>>>>>>> Stashed changes
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
