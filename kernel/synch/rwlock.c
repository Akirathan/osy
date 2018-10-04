/**
 * @file rwlock.c
 *
 * Read/write lock.
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

#include <synch/rwlock.h>


/** Initialize a read/write lock
 *
 * Initialize a read/write lock. There are obviously
 * no waiting readers or writers.
 *
 * @param rwl Read/write lock to initialize.
 *
 */
void rwlock_init (struct rwlock *rwl)
{
	assert (rwl != NULL);
	
	// TODO: Initialize the control structure members
	//       to sane initial values.
	
	list_init (&rwl->read_wait_queue);
	list_init (&rwl->write_wait_queue);
}


/** Clean up a read/write lock
 *
 * Clean up a read/write lock. If the lock is not in the unlocked
 * state (there are readers and/or writers still waiting on the
 * lock), then trigger a kernel panic.
 *
 * @param rwl Read/write lock to clean up.
 *
 */
void rwlock_destroy (struct rwlock *rwl)
{
	assert (rwl != NULL);
	
	if (rwl->state != RWLOCK_UNLOCKED) {
		panic ("Attempt to destroy a read/write lock in use\n");
	}
}


/** Wake up a waiting thread from a wait queue
 *
 * Wake up a single thread from the given wait queue and
 * decreate the variable indicating the number of waiting
 * threads.
 *
 * The purpose of this function is to avoid code repetition.
 *
 * This must be called with disabled interrupts.
 *
 * @param wait_queue  Pointer to a wait queue containing sleeping
 *                    threads.
 * @param num_waiting Pointer to a variable containing the number
 *                    of sleeping threads.
 *
 */
static void rwlock_wakeup (list_t *wait_queue, unsigned int *num_waiting)
{
	// TODO: If the wait queue is not empty, then wake up the first
	//       thread in the wait queue and decrement num_waiting
}


/** Acquire a writer's lock
 *
 * Acquire a writer's lock (exclusive). If the read/write
 * lock is not in the unlocked state (i.e. there is already
 * a writer or there are already some readers in the critical
 * section of the read/write lock), then the writer is blocked.
 *
 * @param rwl Read/write lock to acquire for a writer.
 *
 */
void rwlock_write_lock (struct rwlock *rwl)
{
	assert (rwl != NULL);
	
	// TODO: Execute the rest of the function with disabled interrupts
	
	while (rwl->state != RWLOCK_UNLOCKED) {
		thread_t thread = thread_get_current ();
		
		list_append (&rwl->write_wait_queue, &thread->wait_queue_link);
		rwl->num_write_waiting++;
		
		thread_suspend ();
	}
	
	rwl->state = RWLOCK_LOCKED_WRITE;
}


/** Release a writer's lock
 *
 * Release a writer's exclusive lock. If there are any writers
 * and/or readers waiting on the read/write lock, there are woken
 * up (in this order).
 *
 * @param rwl Read/write to release.
 *
 */
void rwlock_write_unlock (struct rwlock *rwl)
{
	assert (rwl != NULL);
	
	// TODO: Execute the rest of the function with disabled interrupts
	
	if (rwl->state != RWLOCK_LOCKED_WRITE) {
		panic ("Attempt to unlock a read/write lock not locked for writing\n");
	}
	
	rwl->state = RWLOCK_UNLOCKED;
	
	// TODO: First, wake up any waiting writer to avoid writer
	//       starvation.
	
	// TODO: Wake up all waiting readers.
}


/** Acquire a reader's lock
 *
 * Acquire a reader's lock (non-exclusive). Multiple readers
 * cam be in the critical section of a read/write lock. However,
 * if there is already a writer in the critical section of the
 * read/write lock, the reader is blocked.
 *
 * @param rwl Read/write lock to acquire for a reader.
 *
 */
void rwlock_read_lock (struct rwlock *rwl)
{
	// TODO: Execute the rest of the function with disabled interrupts.
	//
	//       Suspend the current thread while the lock is being owned
	//       exclusively by a writer. Otherwise let the reader in.
}


/** Release a readers's lock
 *
 * Release a readers's non-exclusive lock. If there are no more
 * readers in the critical section of the read/write lock and
 * there is a writer waiting on the read/write lock, it is woken
 * up.
 *
 * @param rwl Read/write to release.
 *
 */
void rwlock_read_unlock (struct rwlock *rwl)
{
	assert (rwl != NULL);
	
	// TODO: Execute the rest of the function with disabled interrupts
	
	if (rwl->state != RWLOCK_LOCKED_READ) {
		panic ("Attempt to unlock a read/write lock not locked for reading\n");
	}
	
	assert (rwl->num_readers > 0);
	
	rwl->num_readers--;
	
	/*
	 * Wake up the first waiting writer if there are
	 * no more readers.
	 */
	if (rwl->num_readers == 0) {
		rwl->state = RWLOCK_UNLOCKED;
		rwlock_wakeup (&rwl->write_wait_queue, &rwl->num_write_waiting);
	}
}
