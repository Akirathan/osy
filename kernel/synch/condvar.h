/**
 * @file condvar.h
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

#ifndef CONDVAR_H_
#define CONDVAR_H_


#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <adt/list.h>
#include <synch/mutex.h>


/** Condition variable control structure.
 *
 */
struct condvar {
	/** Number of waiting threads */
	unsigned int num_waiting;
	
	/** Condition variable wait queue.
	 *
	 * Consists of a linked list of threads
	 * waiting at the condition variable. The queue
	 * links threads via their wait_queue_item member.
	 */
	list_t wait_queue;
};


/* Externals are commented with implementation */
extern void condvar_init (struct condvar *cvar);
extern void condvar_destroy (struct condvar *cvar);
extern void condvar_signal (struct condvar *cvar);
extern void condvar_broadcast (struct condvar *cvar);
extern void condvar_wait (struct condvar *cvar, struct mutex *mtx);


#endif /* CONDVAR_H_ */
