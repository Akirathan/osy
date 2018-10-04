/**
 * @file rmutex.h
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

#ifndef RMUTEX_H_
#define RMUTEX_H_


#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <adt/list.h>


/** Recursive mutex control structure.
 *
 */
struct rmutex {
	/** Current owner of the recursive mutex */
	thread_t owner;
	
	/** Lock count */
	unsigned int num_locked;
	
	/** Number of waiting threads */
	unsigned int num_waiting;
	
	/** Mutex wait queue.
	 *
	 * Consists of a linked list of threads
	 * waiting at the mutex. The queue links
	 * threads via their wait_queue_item member.
	 */
	list_t wait_queue;
};


/* Externals are commented with implementation */
extern void rmutex_init (struct rmutex *mtx);
extern void rmutex_destroy (struct rmutex *mtx);
extern void rmutex_lock (struct rmutex *mtx);
extern void rmutex_unlock (struct rmutex *mtx);


#endif /* RMUTEX_H_ */
