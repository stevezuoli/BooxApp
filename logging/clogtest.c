#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define ENABLE_LOG
#include "log.h"

#define NUM_THREADS 4

void* thread_main(void* p)
{
    while (1)
    {
        LOGMSG(LOG_ERROR, 
               "ProcessID = %d, Thread ID = %u.",
               getpid(),
               pthread_self());
        usleep(1000);
    }

    return NULL;
}

int main(int argc, char* argv[])
{
#ifdef ENABLE_LOG
    log_init();
#endif

    pthread_t tid[NUM_THREADS];

    int i;
    for (i=0; i<NUM_THREADS; i++)
    {
        pthread_create(&tid[i], NULL, thread_main, NULL);
    }

    // wait for all children to start up
    sleep(2);

    for (i=0; i<NUM_THREADS; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;   
}

