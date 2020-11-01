#include <stdio.h>
#define ONE 1
#define TWO 2
#define THREE 3


int main() {

    #if COND == ONE
    printf("1\n");

    #elif COND == TWO
    printf("2\n");

    #elif COND == THREE
    printf("3\n");
    #endif

    printf("Done\n");
}