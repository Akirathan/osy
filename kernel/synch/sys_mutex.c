/**
 * @file sys_mutex.c
 *
 * User space mutexes.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <mm/malloc.h>
#include <proc/process.h>

#include <synch/sys_mutex.h>


/** Get user space mutex control structure
 *
 * Convert mutex ID to user space mutex control structure.
 *
 * @param mid Mutex ID.
 *
 * @return User space mutex control structure.
 * @return NULL if the mutex ID is invalid.
 *
 */
static umutex_t umutex_get (unative_t mid)
{
	process_t process = thread_get_process ();
	umutex_t umutex = NULL;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	// TODO: Convert the mutex ID to the actual user space mutex
	//       control structure (umutex_t).
	//
	// Note that the process control structure (process_t) contains
	// a list of valid user space mutexes.
	(void) process;
	
	conditionally_enable_interrupts (state);
	
	return umutex;
}


/** Initialize a mutex
 *
 * Initialize a mutex to the unlocked state.
 *
 * @param mid Place to store the initalized mutex ID (in user memory).
 *
 * @return EOK if the mutex initialization was successful.
 * @return Error code otherwise.
 *
 */
unative_t sys_mutex_init (unative_t *mid)
{
	/*
	 * Check whether it is safe to access the output
	 * argument.
	 */
	
	// TODO: Make sure to check that the mid pointer points to a safely
	//       mapped user memory. The check should be done using
	//       vma_check_user().
	
	// TODO: Allocate the user space mutex control structure
	
	struct umutex *umutex = NULL;
	if (!umutex)
		return ENOMEM;
	
	process_t process = thread_get_process ();
	
	mutex_init (&umutex->mtx);
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	// TODO: Add the newly created user space mutex to the list of mutexes
	//       owned by the current process.
	//
	// Note that the process control structure (process_t) contains
	// a list of valid user space mutexes.
	(void) process;
	
	conditionally_enable_interrupts (state);
	
	(* mid) = (unative_t) umutex;
	return EOK;
}


/** Lock a mutex.
 *
 * Attempt to lock a mutex. If the mutex is already owned by
 * a thread, the current thread is put to sleep until the original
 * owner releases the mutex and wakes it up.
 *
 * @param mid Mutex ID to lock.
 *
 * @return EOK if the mutex locking was successful.
 * @return Error code otherwise.
 *
 */
unative_t sys_mutex_lock (unative_t mid)
{
	// TODO: Implement this syscall
	return EOK;
}


/** Unlock a mutex.
 *
 * Unlock the mutex owned by the current thread. If there is
 * a thread waiting for the mutex, then pass the ownership
 * to that thread and wake it up.
 *
 * If the mutex is being unlocked by a different thread than
 * the owner then kill the thread.
 *
 * @param mid Mutex ID to unlock.
 *
 * @return EOK if the mutex unlocking was successful.
 * @return Error code otherwise.
 *
 */
unative_t sys_mutex_unlock (unative_t mid)
{
	umutex_t umutex = umutex_get (mid);
	if (umutex == NULL)
		return EINVAL;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	if ((umutex->mtx.owner) &&
	    (umutex->mtx.owner != thread_get_current ())) {
		conditionally_enable_interrupts (state);
		thread_finish (NULL);
		
		/* Unreachable */
		return EOK;
	}
	
	// TODO: Call the appropriate kernel function here
	//       (this is not hard).
	
	conditionally_enable_interrupts (state);
	
	return EOK;
}


/** Clean up a mutex
 *
 * Clean up a mutex. If the mutex is currently locked
 * then kill the thread.
 *
 * @param mid Mutex ID to clean up.
 *
 * @return EOK if the mutex cleaning up was successful.
 * @return Error code otherwise.
 *
 */
unative_t sys_mutex_destroy (unative_t mid)
{
	umutex_t umutex = umutex_get (mid);
	if (umutex == NULL)
		return EINVAL;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	if (umutex->mtx.owner) {
		conditionally_enable_interrupts (state);
		thread_finish (NULL);
		
		/* Unreachable */
		return EOK;
	}
	
	list_remove (&umutex->link);
	
	// TODO: Call the appropriate kernel function here
	//       (this is not hard).
	
	conditionally_enable_interrupts (state);
	
	free (umutex);
	
	return EOK;
}
