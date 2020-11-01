#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    printf(2, "This is a test message\n");
    // procdump();
    printf(2, "Before fork:\n");
    ps();
    int pid = fork();
    if (pid == 0) {
        sleep(2);
        printf(2, "Started new process\n");
        sleep(5);
        exit();
    }
    printf(2, "After fork:\n");
    ps();
    sleep(2);
    printf(2, "After sleep:\n");
    ps();
    wait();
    sleep(1000);
    printf(2, "After wait:\n");
    ps();
    exit();
}