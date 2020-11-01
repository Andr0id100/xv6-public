#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf(2, "Invalid argument format\n");
        exit();
    }

    int new_pr = atoi(argv[1]);
    int pid = atoi(argv[2]);

    int old_pr = set_priority(pid, new_pr);
    if (old_pr != -1)
        printf(2, "Changed priority of %d from %d to %d\n", pid, old_pr, new_pr);
    else
        printf(2, "Invalid argument(s)\n");
    exit();

}