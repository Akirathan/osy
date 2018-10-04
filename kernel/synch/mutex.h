/**
 * @file mutex.h
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

#ifndef MUTEX_H_
#define MUTEX_H_


#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <adt/list.h>


/** Mutex control structure.
 *
 */
struct mutex {
	/** Current owner of the mutex */
	thread_t owner;
	
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
extern void mutex_init (struct mutex *mtx);
extern void mutex_destroy (struct mutex *mtx);
extern void mutex_lock (struct mutex *mtx);
extern void mutex_unlock (struct mutex *mtx);


#endif /* MUTEX_H_ */
