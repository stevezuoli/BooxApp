/**
 * Information about the device stack memory
 *
 * This file contains type definitions only and so will not need
 * implementing in the Alien application.
 *
 * @file
 * $Id: alien-stack.h,v 1.12 2009/08/17 12:43:06 roger Exp $
 */
/*  Copyright (C) Picsel, 2004-2008. All Rights Reserved. */
/**
 * @addtogroup TgvMemory
 * @ingroup TgvSystem
 * @{
 */

#ifndef ALIEN_STACK_H
#define ALIEN_STACK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * A configuration value for recovering the stack pointer.
 * This value depends on the ordering of the @c jmp_buf structure
 * for the Device platform. It should be set to the index of the stack
 * pointer within the Device platform specific @c jmp_buf structure
 * during the initialisation by the Alien application integrator.
 *
 * @note Only required when using Picsel_ThreadModel_softThreads().
 */
extern const int AlienPal_stackIndex;

/**
 * A configuration value for recovering amount of stack discarded
 * from a @c setjmp call. It's how many extra bytes of stack is taken
 * up by @c setjmp call. The value depends on the implementation of
 * the @c setjmp for the Device platform. It should be set during
 * the initialisation by the Alien application integrator.
 *
 * @note Only required when using Picsel_ThreadModel_softThreads().
 */
extern const int AlienPal_discardedStack;

/**
 * A direction in which the stack is to be grown. It depends on weither
 * the Device platform's stack goes up or down in the memory.
 */
typedef enum PicselPal_stackDirection
{
    PicselPal_stackGrowsUp = 65539, /**< Grows from lower to higher addresses */
    PicselPal_stackGrowsDown        /**< Grows from higher to lower addresses */
}
PicselPal_stackDirection;

/**
 * Value that shows in which direction the stack grows.
 *
 * The @c AlienPal_stackDirection should be set by the Alien
 * application during the initialisation before calling Picsel library
 * functions. The Alien application integrator should refer to
 * the Device platform implementation to find out weither the stack
 * is growing up or down.
 *
 * @note Only required when using Picsel_ThreadModel_softThreads().
 */
extern const PicselPal_stackDirection AlienPal_stackDirection;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !ALIEN_STACK_H */
