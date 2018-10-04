/**
 * @file rwlock.h
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

#ifndef RWLOCK_H_
#define RWLOCK_H_


#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <adt/list.h>


/** Read/write lock state.
 *
 */
enum rwlock_state {
	/** Unlocked read/write lock
	 *
	 */
	RWLOCK_UNLOCKED = 0,
	
	/** Read/write lock locked for reading (non-exclusive)
	 *
	 */
	
	RWLOCK_LOCKED_READ,
	
	/** Read/write lock locked for writing (exclusive)
	 *
	 */
	RWLOCK_LOCKED_WRITE
};


/** Read/write lock control structure.
 *
 */
struct rwlock {
	/** Current locked state of the read/write lock
	 *
	 */
	enum rwlock_state state;
	
	/** Number of readers inside the critical section
	 *
	 */
	unsigned int num_readers;
	
	/** Number of readers waiting to acquire the lock
	 *
	 */
	unsigned int num_read_waiting;
	
	/** Number of writers waiting to acquire the lock
	 *
	 */
	unsigned int num_write_waiting;
	
	/** Readers wait queue
	 *
	 */
	list_t read_wait_queue;
	
	/** Writers wait queue
	 *
	 */
	list_t write_wait_queue;
};


/* Externals are commented with implementation */
extern void rwlock_init (struct rwlock *rwl);
extern void rwlock_destroy (struct rwlock *rwl);
extern void rwlock_read_lock (struct rwlock *rwl);
extern void rwlock_write_lock (struct rwlock *rwl);
extern void rwlock_write_unlock (struct rwlock *rwl);
extern void rwlock_read_unlock (struct rwlock *rwl);


#endif /* RWLOCK_H_ */
