/**
 *
 * The AlienThread API allows the creation, destruction and switching
 * of native threads.
 *
 * When a thread is created, it should not be run until told to do so by
 * AlienThread_switch.
 *
 * Copyright (C) Picsel, 2006. All Rights Reserved.
 *
 * $Id: alien-thread.c,v 1.1 2008/10/10 06:50:53 maxim Exp $
 *
 * @author Picsel Technologies
 * @file
 */



#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "alien-thread.h"
#include "alien-context.h"

#define DBUGF(x)
/*#define DBUGF(x) printf x*/

/**
 * Information about an actual native thread.
 */
struct AlienThread
{
    pthread_t    wthread;      /*< Handle for this thread         */
    sem_t        semaphore;    /*< Semaphore to control execution */
    bool         shuttingDown; /*< Thread is shutting down        */
    AlienThread *next;         /*< Next thread in list            */
};


/**
 * Data passed to threadStart.
 */
typedef struct AlienThreadSetup
{
    AlienThread           *thread;    /*< This thread                    */
    AlienThread_StartFunc  startFn;   /*< Function to call when started  */
    void                  *startData; /*< Data to pass to startFn        */
}
AlienThreadSetup;

/**
 * Start function for the new thread.  Creates the thread's
 * semaphore and waits on it.  When signalled, calls the
 * function that was provided to AlienThread_create.
 */
static void *threadStart(void *data)
{
    AlienThreadSetup      *setupData = (AlienThreadSetup *)data;
    AlienThread           *thread    = setupData->thread;
    AlienThread_StartFunc  startFn   = setupData->startFn;
    void                  *startData = setupData->startData;
    int                    err;

    /* Free the setup data */
    free(setupData);

    /* Wait for thread to be signalled by AlienThread_switch.
       Don't allow interrupts */
    do
    {
        err = sem_wait(&thread->semaphore);
    } while (err == -1 && errno == EINTR);
    assert(err == 0);

    /* Start the thread properly */
    if (!thread->shuttingDown)
    {
        startFn(startData);
    }

    /* Should never reach here, as the thread should exit in
     * AlienThread_switch.  Just in case, though: */
    (void)sem_destroy(&thread->semaphore);

    return NULL;
}

AlienThread *AlienThread_create(Alien_Context         *ac,
                                AlienThread_StartFunc  startFn,
                                void                  *startData,
                                unsigned long          stackSize)
{
    AlienThread      *thread;
    AlienThreadSetup *setupData;
    sigset_t          newmask;
    sigset_t          oldmask;
    int               reterr;

    stackSize = stackSize; /* Unused, shush compiler */

    /* Allocate the AlienThread structure */
    thread = malloc(sizeof(*thread));
    if( thread == NULL )
    {
        return NULL;
    }
    /* Semaphore to control thread's execution */
    if (sem_init(&thread->semaphore, 0, 0) != 0)
    {
        DBUGF(("sem_init failed: %s\n", strerror(errno)));
        free(thread);
        return NULL;
    }

    /* This flag is set to tell the thread to shut itself down */
    thread->shuttingDown = false;

    /* Add to thread list */
    thread->next = ac->threadList;
    ac->threadList = thread;

    if (startFn != NULL)
    {
        /* Create a wrapper around the startFn/startData.  This allows us
         * to have our own start function (threadStart) and to keep
         * the thread restrained by a semaphore until it's required. */
        setupData = malloc(sizeof(*setupData));
        if (setupData == NULL)
        {
            free(thread);
            (void)sem_destroy(&thread->semaphore);
            return NULL;
        }

        setupData->thread    = thread;
        setupData->startData = startData;
        setupData->startFn   = startFn;

        /* Create the thread. We don't use the stack size - linux allocates
         * stack on demand as it is used. */

        /* block the SIGALRM signal from sending to the newly created thread,
         * signal mask will be inherited by child thread */
        reterr = sigemptyset(&newmask);
        assert (0 == reterr);
        reterr = sigaddset(&newmask, SIGALRM);
        assert (0 == reterr);
        reterr = pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);
        if (reterr != 0)
        {
            DBUGF(("pthread_sigmask failed: %s\n", strerror(reterr)));
            assert("pthread_sigmask failed" == NULL);
            AlienThread_destroy(ac, thread);
            free(setupData);
            return NULL;
        }

        if(pthread_create(&thread->wthread, NULL, threadStart, setupData)
           != 0)
        {
            DBUGF(("pthread_create failed: %s\n", strerror(errno)));
            AlienThread_destroy(ac, thread);
            free(setupData);
            return NULL;
        }

        /* restore the old signal mask of this thread */
        reterr = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
        if (reterr != 0)
        {
            DBUGF(("pthread_create failed: %s\n", strerror(reterr)));
            assert("pthread_sigmask failed" == NULL);
        }
        /* Continue to run until told to switch in AlienThread_switch */
    }
    else
    {
        /* Setting up AlienThread based on current existing thread */
        thread->wthread = pthread_self();
    }

    DBUGF(("Created thread %ld\n", thread->wthread));

    return thread;
}

void AlienThread_destroy(Alien_Context *ac,
                         AlienThread   *thread)
{
    AlienThread *threadList = NULL;

    if( ( ac == NULL ) || ( thread == NULL ) )
    {
        return;
    }
    DBUGF(("Destroy thread %ld\n", thread->wthread));

    /* Remove from the thread list */
    threadList = ac->threadList;
    if( threadList != NULL )
    {
        if( threadList == thread )
        {
            /* Want to remove start of list */
            ac->threadList = ac->threadList->next;
        }
        else
        {
            /* Search for the thread we want to remove */
            while( ( threadList != NULL ) &&
                   ( threadList->next != thread ) )
            {
                threadList = threadList->next;
            }

             /* Should be in the list! */
            assert( threadList != NULL );
            assert( threadList->next != NULL );

            if( ( threadList != NULL ) &&
                ( threadList->next != NULL ) )
            {
                threadList->next = threadList->next->next;
            }
        }
    }

    if (thread != NULL)
    {
        if (thread == ac->mainThread)
        {
            (void)sem_destroy(&thread->semaphore);
            ac->mainThread = NULL;
        }
        else
        {
            /* Signal this thread's semaphore so it can
             * run to completion */
            thread->shuttingDown = true;
            sem_post(&thread->semaphore);

            /* Wait for the thread to finish */
            pthread_join(thread->wthread, NULL);
        }
    }

    free( thread );
}

void AlienThread_switch(Alien_Context *ac,
                        AlienThread   *thread)
{
    AlienThread *oldThread;
    int          err;

    /* A NULL
     * thread indicates that the main thread is to be
     * signalled.
     */
    if (thread == NULL)
        thread = ac->mainThread;

    oldThread = ac->currentThread;
    assert(oldThread != NULL);

    ac->currentThread = thread;

    DBUGF(("Switch to thread %ld\n", thread->wthread));

    /* Signal the new thread to start running. */
    sem_post(&thread->semaphore);

    /* Block the current thread from running. */
    DBUGF(("Thread %ld blocking\n", oldThread->wthread));

    /* Don't allow interrupts */
    do
    {
        err = sem_wait(&oldThread->semaphore);
    } while (err == -1 && errno == EINTR);
    assert(err == 0);

    DBUGF(("Thread %ld continuing\n", oldThread->wthread));

    if (oldThread->shuttingDown)
    {
        DBUGF(("Thread %ld exitting\n", oldThread->wthread));
        /* Shut down this thread */
        pthread_exit(0);
    }
}

unsigned long AlienThread_getFreeStack(Alien_Context *ac)
{
    ac = ac; /* Unused, shush compiler */

    /* FIXME: To do.  For the moment, say we've got
     * plenty of space free. */
    return 1024 * 1024;
}

void AlienThread_abort(Alien_Context *ac)
{
    if(ac == NULL)
    {
        return;
    }

    /* Destroy threads */
    while (ac->threadList != NULL)
    {
        AlienThread_destroy(ac, ac->threadList);
    }
}
