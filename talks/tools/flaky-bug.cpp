#include <cstdio>
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) {
    srand(time(NULL));
    if (rand() % 50 == 0) {
        volatile int * p = 0;
        *p = 1;
    }

    printf("Done\n");
} 