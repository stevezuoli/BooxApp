#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const int MALLOC_SIZE = 100 * 1024 * 1024;
static const int VALUE_A = 0x0;
static const int VALUE_B = 0xff;

void test2(int size)
{
    unsigned char* data = NULL;
    int i = 0, j = 0;
    int count = MALLOC_SIZE / size;
    unsigned char** p = (unsigned char **)malloc(count * sizeof(unsigned char **));

    // step1. alloc.
    for(i = 0; i < count; ++i)
    {
        data = (unsigned char*)malloc(size);
        p[i] = data;
    }

    // step2. update value.
    for(i = 0; i < count; ++i)
    {
        data = p[i];
        memset(data, VALUE_A, size);
    }

    // step3. verify
    for(i = 0; i < count; ++i)
    {
        data = p[i];
        for(j = 0; j < size; ++j)
        {
            assert(data[j] == VALUE_A);
            data[j] = VALUE_B;
        }
    }

    // step4. reverse
    for(i = count - 1; i >=0; --i)
    {
        data = p[i];
        memset(data, VALUE_B, size);
    }

    for(i = count - 1; i >=0 ; --i)
    {
        data = p[i];
        for(j = 0; j < size; ++j)
        {
            assert(data[j] == VALUE_B);
            data[j] = VALUE_A;
        }
    }

    // Last round.
    for(i = count - 1; i >=0 ; --i)
    {
        data = p[i];
        for(j = 0; j < size; ++j)
        {
            if (data[j] != VALUE_A)
            {
                printf("block %d data[%d] %d\n", i, j, data[j]);
                printf("block %d data[%d] %d\n", i, j, data[j]);
                assert(0);
            }
        }
    }

    for(i = count - 1; i >=0 ; --i)
    {
         free(p[i]);
    }
    free(p);
}


int main(int argc, char* argv[])
{
    int size = 64;
    if (argc == 2)
        sscanf(argv[1], "%d", &size);

    while (1)
    {
        test2(size);
        printf("done!\n");
    }

    return 0;
}
