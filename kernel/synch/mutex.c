/**
 * @file mutex.c
 *
 * Mutexes.
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

#include <synch/mutex.h>


/** Initialize a mutex
 *
 * Initialize a mutex to the unlocked state.
 *
 * @param mtx Mutex to initialize.
 *
 */
void mutex_init (struct mutex *mtx)
{
	assert (mtx != NULL);
	
	mtx->owner = NULL;
	mtx->num_waiting = 0;
	plist_init (&mtx->wait_queue);
}


/** Clean up a mutex
 *
 * Clean up a mutex. If the mutex is currently locked
 * then trigger a kernel panic.
 *
 * @param mtx Mutex to clean up.
 *
 */
void mutex_destroy (struct mutex *mtx)
{
	assert (mtx != NULL);
	
	if (mtx->owner) {
		panic ("Request to destroy a locked mutex.");
	}
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
void mutex_lock (struct mutex *mtx)
{
	assert (mtx != NULL);
	
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU).
	
	if (mtx->owner) {
		
		// TODO: Someone is already holding the mutex.
		//       Add current thread to the wait queue and
		//       put it to sleep.
		
		mtx->num_waiting++;
	} else {
		mtx->owner = thread_get_current ();
	}
}


/** Unlock a mutex.
 *
 * Unlock the mutex owned by the current thread. If there is
 * a thread waiting for the mutex, then pass the ownership
 * to that thread and wake it up.
 *
 * If the mutex is being unlocked by a different thread than
 * the owner then trigger a kernel panic.
 *
 * @param mtx Mutex to unlock.
 *
 */
void mutex_unlock (struct mutex *mtx)
{
	assert (mtx != NULL);
	
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU)
	
	if (mtx->owner) {
		
		// TODO: Make sure to check that the thread that
		//       is locking this mutex is the owner of this
		//       mutex.
		
		if (! plist_empty (&mtx->wait_queue)) {
			assert (mtx->num_waiting > 0);
			
			/*
			 * Remove the first list entry from the wait queue.
			 */
			
			struct plist *item = plist_pop (&mtx->wait_queue);
			thread_t thread = plist_item (item, struct thread, wait_queue_link);
			
			// TODO: Remove the following line
			(void) thread;
			
			// TODO: Pass mutex ownership to the waiting thread
			//       and wake up the sleeper.
			
			mtx->num_waiting--;
		} else {
			// TODO: Handle the case when nobody is waiting
			//       on the mutex.
		}
		
	} else {
		/*
		 * Unlocking a mutex that is not locked.
		 * Ignoring.
		 */
	}
}
