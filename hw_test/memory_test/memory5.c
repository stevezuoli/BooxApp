#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ALLOC_SIZE      1024
#define RECURSIVE_TIMES 1024

int foo()
{
    static int recursive_times = 0;
    char data[ALLOC_SIZE];
    int i=0;

    if (recursive_times++ > RECURSIVE_TIMES)
    {
        recursive_times = 0;
        return 0;
    }

    printf("About to memset: recursive_times = %d\n", recursive_times);
    for (i=0; i<1024; i++)
    {
        memset(data, 0, ALLOC_SIZE);
        memset(data, 0xFF, ALLOC_SIZE);
    }

    printf("About to call foo...\n");
    return foo();
}

int main(int argc, char* argv[])
{
    while (1)
    {
        foo();
    }

    return 0;
}

