#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const int MALLOC_SIZE = 96 * 1024 * 1024;

void test2(int size)
{
    unsigned char* data = NULL;
    int i = 0;
    while (1)
    {
        data = (unsigned char*)malloc(size);

        // Step 1: Write 0 &
        memset(data, 0, size);
        for(i = 0; i < 1000; ++i)
        {
            memset(data, 0xff, size);
            memset(data, 0, size);
        }
        // memset(data, 0xff, MALLOC_SIZE);
        free(data);
        data = 0;
        printf("done!\n");
    }
}


int main(int argc, char* argv[])
{
    int size = MALLOC_SIZE;
    if (argc == 2)
        sscanf(argv[1], "%d", &size);

    test2(size);

    return 0;
}
