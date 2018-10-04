/**
 * @file condvar.c
 *
 * Condition variables.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2016
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */


#include <include/shared.h>
#include <include/c.h>

#include <lib/debug.h>
#include <synch/mutex.h>

#include <synch/condvar.h>


/** Initialize a condition variable
 *
 * Initialize a condition variable. There are obviously
 * no waiting threads.
 *
 * @param cvar Condition variable to initialize.
 *
 */
void condvar_init (struct condvar *cvar)
{
	// TODO
}


/** Clean up a condition variable
 *
 * Clean up a condition variable. If there are threads still
 * waiting on the condition variable, then trigger a kernel panic.
 *
 * @param cvar Condition variable to clean up.
 *
 */
void condvar_destroy (struct condvar *cvar)
{
	// TODO
}


/** Wake up a waiting thread
 *
 * Wake up a single thread waiting on the condition
 * variable. This function contains the shared functionality
 * of condvar_signal() and condvar_broadcast().
 *
 * This must be called with disabled interrupts.
 *
 * @param cvar Condition variable whose thread to wake up.
 *
 */
static void condvar_wakeup (struct condvar *cvar)
{
	// TODO
}


/** Signal condition variable and wake up a waiting thread
 *
 * Signal the condition variable and wake up a single
 * thread in the wait queue of the condition variable.
 *
 * @param cvar Condition variable to signal.
 *
 */
void condvar_signal (struct condvar *cvar)
{
	// TODO
	(void) condvar_wakeup;
}


/** Signal condition variable and wake up all waiting threads
 *
 * Signal the condition variable and wake up all threads
 * in the wait queue of the condition variable.
 *
 * @param cvar Condition variable to signal.
 *
 */
void condvar_broadcast (struct condvar *cvar)
{
	// TODO
}


/** Wait on a condition variable
 *
 * Relinquish the mutex and wait for a signal on the condition
 * variable. The mutex is acquired again after we are woken
 * up.
 *
 * @param cvar Condition variable to wait on.
 * @param mtx  Mutex to unlock atomically.
 *
 */
void condvar_wait(struct condvar *cvar, struct mutex *mtx)
{
	// TODO
}
