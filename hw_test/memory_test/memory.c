#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const int MALLOC_SIZE = 96 * 1024 * 1024;
static const int VALUE_A = 0x0;
static const int VALUE_B = 0xff;

int main(int argc, char* argv[])
{
    unsigned char* data = NULL;
    int i = 0;

    while (1)
    {
        data = (unsigned char*)malloc(MALLOC_SIZE);


        for(i = 0; i < 800; ++i)
        {
            // Step 1: Write VALUE_A
            memset(data, VALUE_A, MALLOC_SIZE);

            // Verify if the memory is filled with VALUE_A.
            for (i = 0; i < MALLOC_SIZE; i++)
            {
                assert (data[i] == VALUE_A);
                data[i] = VALUE_B;
            }

            // Step 2: Write VALUE_B
            // Verify if the memory is filled with VALUE_B.
            for (i = MALLOC_SIZE-1; i >= 0; i--)
            {
                assert (data[i] == VALUE_B);
                data[i] = VALUE_A;
            }

            for (i = 0; i < MALLOC_SIZE; i++)
            {
                assert(data[i] == VALUE_A);
            }
        }
        printf("done and free!\n");
        free(data);
        data = 0;
    }

    return 0;
}
