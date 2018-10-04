/**
 * @file mutex.c
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

#include <syscall.h>

#include <mutex.h>


/** Initialize a mutex
 *
 * Initialize a mutex to the unlocked state.
 *
 * @param mtx Mutex to initialize.
 *
 */
int mutex_init (struct mutex *mtx)
{
	// TODO: Call the SYS_MUTEX_INIT syscall
	//       (the first argument is an output argument, pointing
	//       to the memory location where to store the mutex ID)
	return EOK;
}


/** Clean up a mutex
 *
 * Clean up a mutex. If the mutex is currently locked
 * then kill the thread.
 *
 * @param mtx Mutex to clean up.
 *
 */
int mutex_destroy (struct mutex *mtx)
{
	// TODO: Call the SYS_MUTEX_DESTROY syscall
	//       (simply pass the mutex ID)
	return EOK;
}


/** Lock a mutex.
 *
 * Attempt to lock a mutex. If the mutex is already owned by
 * a thread, the current thread is put to sleep until the original
 * owner releases the mutex and wakes it up.
 *
 * @param mtx Mutex to lock.
 *
 */
int mutex_lock (struct mutex *mtx)
{
	// TODO: Call the SYS_MUTEX_LOCK syscall
	//       (simply pass the mutex ID)
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
 * @param mtx Mutex to unlock.
 *
 */
int mutex_unlock (struct mutex *mtx)
{
	// TODO: Call the SYS_MUTEX_UNLOCK syscall
	//       (simply pass the mutex ID)
	return EOK;
}
