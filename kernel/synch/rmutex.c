/**
 * @file rmutex.c
 *
 * Recursive mutexes.
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

#include <lib/debug.h>

#include <synch/rmutex.h>


/** Initialize a recursive mutex
 *
 * Initialize a recursive mutex to the unlocked state.
 *
 * @param mtx Recursive mutex to initialize.
 *
 */
void rmutex_init (struct rmutex *mtx)
{
	assert (mtx != NULL);
	
	mtx->owner = NULL;
	mtx->num_locked = 0;
	mtx->num_waiting = 0;
	list_init (&mtx->wait_queue);
}


/** Clean up a recursive mutex
 *
 * Clean up a recursive mutex. If the mutex is currently locked
 * then trigger a kernel panic.
 *
 * @param mtx Recursive mutex to clean up.
 *
 */
void rmutex_destroy (struct rmutex *mtx)
{
	assert (mtx != NULL);
	
	if (mtx->owner) {
		panic ("Request to destroy a locked recursive mutex.");
	}
}


/** Lock a recursive mutex.
 *
 * Attempt to lock a recursive mutex. If the mutex is already owned
 * by a thread, the current thread (unless it is not the owner of
 * the mutex) is put to sleep until the original owner releases
 * the mutex and wakes it up.
 *
 * If the locking thread is the owner of the recursive mutex, the
 * thread is not put to sleep, but the lock count is incremented.
 *
 * @param mtx Recursive mutex to lock.
 *
 */
void rmutex_lock (struct rmutex *mtx)
{
	assert (mtx != NULL);
	
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU).
	
	if (mtx->owner) {
		thread_t thread = thread_get_current ();
		
		if (mtx->owner == thread) {
			// TODO: Someone is already holding the mutex.
			//       Handle the case when it is the same thread.
		} else {
			// TODO: Someone is already holding the mutex.
			//       Add current thread to the wait queue and
			//       put it to sleep.
			mtx->num_waiting++;
		}
	} else {
		mtx->owner = thread_get_current ();
		mtx->num_locked = 1;
	}
}


/** Unlock a recursive mutex.
 *
 * Unlock the recursive mutex owned by the current thread. If the
 * number of unlock operations balances out the number of lock
 * operations and if there is a thread waiting for the mutex,
 * then pass the ownership to that thread and wake it up.
 *
 * If the mutex is being unlocked by a different thread than
 * the owner then trigger a kernel panic.
 *
 * @param mtx Recursive mutex to unlock.
 *
 */
void rmutex_unlock (struct rmutex *mtx)
{
	assert (mtx != NULL);
	
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU)
	
	if (mtx->owner) {
		assert (mtx->num_locked > 0);
		
		// TODO: Make sure to check that the thread that
		//       is locking this mutex is the owner of this
		//       mutex.
		
		mtx->num_locked--;
		
		if (mtx->num_locked == 0) {
			link_t *link = list_pop (&mtx->wait_queue);
			if (link != NULL) {
				assert (mtx->num_waiting > 0);
				
				/*
				 * Remove the first list entry from the wait queue.
				 */
				
				thread_t thread = list_item (link, struct thread, wait_queue_link);
				
				// TODO: Remove the following line
				(void) thread;
				
				// TODO: Pass mutex ownership to the waiting thread
				//       and wake up the sleeper. Make sure to set
				//       the correct value of mtx->num_locked.
				
				mtx->num_waiting--;
			} else {
				// TODO: Handle the case when nobody is waiting
				//       on the mutex.
			}
		}
		
	} else {
		/*
		 * Unlocking a mutex that is not locked.
		 * Ignoring.
		 */
	}
}
