#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const int MALLOC_SIZE = 16 * 1024;

void test2()
{
    unsigned int* data = NULL;
    int i = 0;
    while (1)
    {
        data = (unsigned int*)malloc(MALLOC_SIZE);

        // Step 1: Write 0 &
        for(i = 0; i < MALLOC_SIZE / sizeof(int); ++i)
        {
            if (i % 2)
            {
                data[i] =  0xffffffff;
            }
            else
            {
                data[i] = 0;
            }
        }

        for (i = 0; i < MALLOC_SIZE / sizeof(int); i++)
        {
            if (i % 2)
            {
                assert(data[i] == 0xffffffff);
            }
            else
            {
                assert(data[i] == 0);
            }
        }
        free(data);
        data = 0;
    }
}


int main(int argc, char* argv[])
{
    test2();

    return 0;
}
