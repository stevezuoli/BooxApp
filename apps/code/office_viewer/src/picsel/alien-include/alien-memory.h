/**
 * Memory allocation API for Alien platform abstraction.
 *
 * These functions must be implemented by the application integrator, before
 * the application can be linked with the Picsel library.
 *
 * @file
 * $Id: alien-memory.h,v 1.18 2008/12/11 16:34:19 roger Exp $
 */
/* Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @defgroup TgvMemory Memory Allocation
 * @ingroup TgvSystem
 *
 * Picsel relies on memory in the device being reserved for it, and allows
 * great flexibility in the organisation of this, through the functions in
 * this group. Normally, these functions are implemented by the alien
 * application to use the device operating system's memory management
 * features.
 * @{
 */

#ifndef ALIEN_MEMORY_H
#define ALIEN_MEMORY_H

#include "alien-types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Allocate a block of memory for Picsel to use.
 *
 * This requests a block of memory from the device operating system's
 * heap, for use by the Picsel library.
 *
 * Picsel also has an internal heap memory manager which can be configured
 * by the alien application in two modes.
 *
 * 1. Normally, the Picsel library will call this function only once,
 * during initialisation, and will accept almost any memory allocation
 * offered by the alien application. This can be called the "single heap
 * block" method.
 *
 * 2. If the @ref PicselApp_ExpandingHeap flag was specified to
 * PicselApp_start() when the Picsel library was initialised, then
 * this function will be called repeatedly to request additional
 * memory of specific sizes.
 *
 * The Picsel library can tolerate a range of allocation sizes, with
 * a variable effect on performance and processing capacity. Please
 * see the Integration_Guide for more advice for your product. Some can
 * work with 3Mb, others may need 10Mb. The Picsel library will use the
 * space it is given with its internal heap memory manager, and will
 * degrade gracefully if space is not allocated.
 *
 * In the single heap block mode, the @c size parameter should be
 * treated as an output parameter, and the alien application should
 * allocate as much memory as it wishes the Picsel library to use
 * throughout its session.
 *
 * With an expanding heap, this function will be called multiple
 * times to request additional memory.  The @c size parameter in this case
 * will hold the minimum size that must be allocated.  The application may
 * allocate more than that in a single allocation, or nothing, but not a
 * smaller block.
 *
 * @param[in,out] size Size of the memory block, in bytes.
 *                     As an input parameter in the expanding heap mode,
 *                     @c size is the suggested size of the block
 *                     requested by Picsel but may be increased by the
 *                     alien application. As an
 *                     input parameter in the single heap block mode,
 *                     @c size is not important; the alien application
 *                     should be implemented with a pre-defined block size.
 *                     Upon returning, this must output the size (in bytes)
 *                     of the memory block that has actually been allocated.
 *
 * @return      Pointer to start of allocated block of memory,
 *              or @c NULL if no memory was available for Picsel.
 * @post        The memory pointed to by the return value must be
 *              reserved for the exclusive use of the Picsel library.
 *              The Picsel library will call AlienMemory_free() to release
 *              it, later.
 * @product     This function must be implemented by the alien application
 *              for all Picsel TGV products.
 * @implement   This function can usually be implemented by limiting the
 *              value of @c size according to the device manufacturer's
 *              policy, and then calling the system malloc() function.
 * @see AlienMemory_free() AlienMemory_mallocStack()
 */
void *AlienMemory_malloc(unsigned int *size);


/**
 * Release a block of memory allocated with AlienMemory_malloc().
 *
 * This releases a block of memory from the device operating system's
 * heap, which was previously allocated to the Picsel library using
 * AlienMemory_malloc(), It must be implemented by the
 * alien application for use by Picsel.
 *
 * @param[in] mem Pointer to start of block of memory to be freed.
 *            The alien application should know the size of this block.
 * @pre       The Picsel library has previously called
 *            AlienMemory_malloc() which returned @c mem.
 * @post      The Picsel library will not use the memory pointed to
 *            by @c mem.
 * @product   This function must be implemented by the alien application
 *            for all Picsel TGV products.
 * @implement This function can usually be implemented by calling the
 *            system free() function.
 * @see       AlienMemory_malloc()
 */
void AlienMemory_free(void *mem);


/**
 * Allocate memory suitable for use by Picsel as a stack.
 *
 * This function is similar to AlienMemory_malloc(). It may be necessary
 * to use a special memory area for thread stacks (this is true on
 * Microsoft Windows and with some versions of Linux pthreads). If there is
 * no requirement for this on the current platform, this function does not
 * need to be implemented by the alien application.
 *
 * The Picsel library uses soft-threading internally to perform multiple
 * tasks within a single execution context, and switches between these
 * using multiple stacks and CPU register values. See @ref TgvHost_Thread
 * for related information.
 *
 * The memory allocated will be used for one stack. This function will be
 * called repeatedly to allocate memory for multiple thread stacks.
 *
 * @product This function is not normally used. If the alien application
 *          is to be used with soft threading on a platform which
 *          distinguishes between normal memory and stack memory, please
 *          contact your Picsel support representative for a build that
 *          supports this. On such platforms, it must be implemented for
 *          all Picsel libraries.
 *
 * @param[in] size Requested stack size, in bytes. Unlike
 *            AlienMemory_malloc(), the block allocated should
 *            be exactly this size.
 * @return    Pointer to bottom of the newly allocated stack memory,
 *            or NULL if the memory could not be allocated.
 * @post      The memory must be
 *            reserved for the exclusive use of the Picsel library,
 *            and must be suitable for use as a process stack.
 *            The Picsel library will call AlienMemory_freeStack()
 *            to release it, later.
 * @see       AlienMemory_freeStack(), AlienMemory_malloc()
 */
void *AlienMemory_mallocStack(int size);


/**
 * Release memory which was allocated with AlienMemory_mallocStack().
 *
 * This is similar to AlienMemory_free() but is called by Picsel for
 * memory blocks that were claimed using AlienMemory_mallocStack().
 *
 * @param[in] mem Pointer to the block of memory to be freed.
 * @pre       The block of memory that was allocated to Picsel by
 *            AlienMemory_mallocStack().
 * @post      The Picsel library will not use the memory pointed to by
 *            @c mem after this.
 * @product   This function is only needed if AlienMemory_mallocStack()
 *            is also needed.
 * @see       AlienMemory_mallocStack()
 */
void AlienMemory_freeStack(void *mem);

/**
 * Get the Picsel_Context.
 *
 * This function is used to obtain the current @ref Picsel_Context.
 * This is an area of memory used by Picsel to store its state and
 * working data. It must not be used by the alien application. It
 * is defined by Picsel using AlienEvent_setPicselContext().
 *
 * @return    The Picsel Context pointer that was set by
 *            AlienEvent_setPicselContext()
 * @pre       The Picsel library will call AlienEvent_setPicselContext()
 *            before this.
 * @product   This function must be implemented by the alien application
 *            for all Picsel TGV products.
 * @runtime   The Picsel library will call this function frequently. It is
 *            @em essential that it returns quickly. Even a delay of
 *            5us will slow down Picsel performance by approximately 90% !
 * @implement The alien application may be able to hold a global variable
 *            for the Picsel_Context and can simply return it.
 * @see       AlienEvent_setPicselContext()
 */
Picsel_Context *AlienMemory_getPicselContext(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_MEMORY_H */
