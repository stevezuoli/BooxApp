/**
 * Device platform threading for concurrent processing.
 *
 * Control and creation of threads for concurrent processing of multiple
 * threads of control, within the shared memory of a single process, using
 * the Device Platform's own threading system.
 *
 * If Picsel_ThreadModel_alienThreads() is selected as
 * a threading model for the Picsel library then functions defined in
 * this file must be implemented by the Alien application integrator
 * before the application can be linked with the Picsel library.
 * The implementation is not required if Picsel's own soft-threading
 * system is used.
 *
 *
 * @file
 * $Id: alien-thread.h,v 1.11 2008/12/11 16:34:19 roger Exp $
 */
/* Copyright (C) Picsel, 2006-2008. All Rights Reserved. */
/**
 * @defgroup TgvThreading Threading
 * @ingroup TgvSystem
 *
 * The Picsel library is multi-threaded. This allows many components
 * within its sophisticated architecture to run concurrently, even on
 * a single-processor device. It has been designed to be efficient in
 * CPU time, responsive to user input even while processing, and compliant
 * with the demands placed upon it by device operating systems, such
 * as limits imposed by watchdog timers.
 *
 * There are two ways in which the library can manage these threads: soft
 * threading, and alien threading. The threading model must be chosen on
 * startup. See PicselApp_start().
 *
 * The soft threading model means that the Picsel library will handle its threading
 * internally, and will run within one platform thread.
 *
 * The main advantage of using soft threads is that no alien implementation
 * is required. It is recommended to use soft threading for most platforms.
 * However, soft threading does not work on all platforms. This may mean that,
 * on some platforms, a soft threading build will crash on startup.
 * In such cases, alien threads must be used.
 *
 * The Alien threading model means that the Picsel library requires native thread
 * support in order to run. To get an alien thread build working, the functions
 * in the alien-thread.h file will need to be implemented by the Alien
 * application engineer. They will be called by Picsel, to request threading
 * services from the Alien application.
 *
 * All resources (file handles, etc) should be accessible from every
 * thread and each thread show run until a context switch is requested
 * from the Picsel library.
 *
 * @{
 */

#ifndef ALIEN_THREAD_H
#define ALIEN_THREAD_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * An opaque type for the Device platform (alien) thread data
 * that the Alien application may wish to store. The Alien
 * application integrator is expected to define a struct
 * @c AlienThread containing the information about
 * an actual native thread they require. It is normally
 * the Device Platform thread handle and semaphore to control
 * the thread execution. This structure is returned by
 * AlienThread_create() and passed to AlienThread_switch() and
 * AlienThread_destroy(). Picsel library will not access
 * the structure.
 */
typedef struct AlienThread AlienThread;

/**
 * An opaque handle pointing to a function within the Picsel library
 * which will run in this thread. This function will do the main
 * processing required by this thread. It will run for a long time
 * until this function exits or the thread is terminated.
 *
 * @post This is passed to AlienThread_create(), and must be called the
 * first time that AlienThread_switch() is called for the thread.
 *
 * @param[in] startData  A pointer to the structure as passed to
 *                       @ref AlienThread_create()
 */
typedef void (*AlienThread_StartFunc)(void *startData);


/**
 * Create a native thread.
 *
 * This function is called by Picsel library to request the Alien application
 * to create a thread.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[in] startFn       Function to be called when thread starts running.
 * @param[in] startData     Pointer to be passed to @c startFn.
 * @param[in] stackSize     Stack size required, in bytes
 *
 * @return handle for new thread, or NULL on failure.
 *
 * @post The @c startFn is the entry point function for the thread.
 * It must not be called until the first time AlienThread_switch()
 * is called on the returned thread handle.
 *
 * @post AlienThread_create() will be called by the Picsel library once
 * to allow the Alien application to store information about the main TGV
 * thread. In this case, @c startFn will be NULL, and no extra thread
 * should be created.
 *
 * @implement @code
 * AlienThread *AlienThread_create(Alien_Context         *ac,
 *                                 AlienThread_StartFunc  startFn,
 *                                 void                  *startData,
 *                                 unsigned long          stackSize)
 * {
 *     // Allocate the AlienThread structure
 *     thread = malloc(sizeof(*thread));
 *     ...
 *
 *     // Semaphore to control thread's execution
 *     thread->semaphore = CreateSemaphore(NULL, 0, 1, NULL);
 *     ...
 *
 *     if (startFn != NULL)
 *     {
 *         ...
 *         // create thread
 *         thread->wthread = Thread_create( startFn, startData, stackSize);
 *     }
 *     else
 *     {
 *         // Setting up AlienThread based on current existing thread
 *         thread->wthread = Thread_getCurrentThreadId();
 *     }
 *
 *     // Add to thread list
 *     thread->next = ac->threadList;
 *     ac->threadList = thread;
 *
 *     return thread;
 * }
 * @endcode
 *
 */
AlienThread *AlienThread_create(Alien_Context         *alienContext,
                                AlienThread_StartFunc  startFn,
                                void                  *startData,
                                unsigned long          stackSize);

/**
 * Destroy a native (alien) thread.
 *
 * The thread to be destroyed may be dormant or may have already
 * exited.
 *
 * @param[in] alienContext  See PicselApp_start()
 * @param[in] thread        Thread to be destroyed.
 *
 * @post This function will not be called if the Picsel library has suffered
 * a fatal error and has already called AlienThread_abort().
 *
 * @implement @code
 * void AlienThread_destroy(Alien_Context *ac, AlienThread   *thread)
 * {
 *     ...
 *
 *     threadList = ac->threadList;
 *     if( threadList != NULL )
 *     {
 *         // search for thread we need to remove
 *         while( ( threadList != NULL ) && ( threadList->next != thread ))
 *         {
 *             threadList = threadList->next;
 *         }
 *
 *         ...
 *
 *         // remove the thread from the thread list
 *         if( ( threadList != NULL ) && ( threadList->next != NULL ))
 *         {
 *             threadList->next = threadList->next->next;
 *         }
 *
 *         ...
 *
 *         if (thread != NULL)
 *         {
 *             // if this is the main thread, then destroy the semaphore
 *             if( thread == ac->mainThread)
 *             {
 *                 Semaphore_destroy(&thread->semaphore);
 *             }
 *             else
 *             {
 *                 // signal the semaphore for this thread
 *                 Semaphore_post(&thread->semaphore);
 *
 *                 // and wait for the thread to complete
 *                 Thread_join(thread->wthread, NULL);
 *             }
 *         }
 *
 *         ...
 *
 *         // free the resources allocated to this thread
 *         free( thread);
 *     }
 *
 * }
 * @endcode
 *
 */
void AlienThread_destroy(Alien_Context *alienContext, AlienThread *thread);

/**
 * Switch to the given thread.
 *
 * The calling thread is blocked and context is switched to the specified
 * thread.
 *
 * @param[in] alienContext  See PicselApp_start().
 * @param[in] thread        Thread to switch to. If @c thread is NULL,
 *                          the Alien application should switch to
 *                          the main Picsel thread, which was identified
 *                          the first time that AlienThread_create() was
 *                          called.
 *
 * @implement @code
 * void AlienThread_switch(Alien_Context *ac, AlienThread   *thread)
 * {
 *     ...
 *
 *     oldThread = ac->currentThread;
 *     ac->currentThread = thread;
 *
 *     // Signal the new thread to allow it tostart running
 *     Semaphore_post(&thread->semaphore);
 *
 *     //wait on the semaphore of the current thread
 *     do
 *     {
 *         err = Semaphore_wait(&oldThread->semaphore));
 *     } while( err == -1 && errno == EINTR);
 *
 *     ...
 *
 * }
 * @endcode
 *
 */
void AlienThread_switch(Alien_Context *alienContext, AlienThread *thread);


/**
 * Get the amount of stack space (in bytes) remaining in the current thread.
 *
 * @param[in] alienContext  See PicselApp_start()
 *
 * @return the amount of stack still available for the current thread to use
 */
unsigned long AlienThread_getFreeStack(Alien_Context *alienContext);


/**
 * Notify the Alien application that a fatal error has been detected
 * by the Picsel library.
 *
 * @param[in] alienContext  See PicselApp_start()
 *
 * @post This function will only be called by the Picsel library from
 * the main thread. The Alien application must shut down all created
 * native threads. On returning, the Picsel library will finish
 * shutting down and will then call AlienError_fatal(). The Alien
 * application integrator is responsible for ensuring that any
 * remaining thread handles are closed.
 *
 * @implement @code
 * void AlienThread_abort(Alien_Context *ac)
 * {
 *     if(ac == NULL)
 *     {
 *         return;
 *     }
 *
 *     // Destroy threads
 *     while (ac->threadList != NULL)
 *     {
 *         AlienThread_destroy(ac, ac->threadList);
 *     }
 * }
 * @endcode
 */
void AlienThread_abort(Alien_Context *alienContext);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_THREAD_H */
