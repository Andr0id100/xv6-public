#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int pid = fork();

    if (pid == 0) {
        int res = exec(argv[1], argv+1);
        // If this statement is executed then some problem occured with exec
        if (res) {
            printf(2, "Some error with exec\nc");
            exit();
        }
        
    }
    int wtime, rtime;
    int wait_ret = waitx(&wtime, &rtime);
    printf(2, "Wait returned with value: %d\n", wait_ret);
    printf(2, "Waiting Time: %d\nRunning Time: %d\n", wtime, rtime);
    exit();
}