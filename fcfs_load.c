#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char* argv[]) {
    int process_count = 10;
    printf(2, "Testing FCFS:\n");
    for(int i=0;i<process_count;i++) {
        int pid = fork();
        if (pid == 0) {
            printf(2, "Started process %d\n", i);
            sleep(1000);
            // printf(2, "\nProcess %d done\n\n", i);
            exit();
        }
        sleep(5);
    }
    sleep(5);
    // ps();
    for (int i=0;i<process_count;i++) {
        wait();
        printf(2, "Reaped a process\n");
    }
    ps();

    printf(2, "DONE\n");
    exit();
}