/**
 * @file sem.h
 *
 * Semaphores.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef SEM_H_
#define SEM_H_


#include <adt/list.h>


/** Semaphore control structure.
 *
 */
struct semaphore {
	/*
	 * The value of the semaphore.
	 */
	int value;
	
	/*
	 * The value of the semaphore will never exceed the
	 * limit. Useful for implementing mutexes and condition
	 * variables.
	 */
	int limit;
	
	/*
	 * The number of waiting threads.
	 */
	unsigned int num_waiting;
	
	/*
	 * Semaphore wait queue. Consists of a linked list
	 * of thread waiting at the semaphore. The queue links
	 * thread via their wq_item member.
	 */
	list_t wait_queue;
};


/* Externals are commented with implementation */
extern void sem_init (struct semaphore *sem, const int value);
extern void sem_init_limit (struct semaphore *sem, const int value,
    const int limit);
extern void sem_destroy (struct semaphore *sem);
extern int sem_get_value (struct semaphore *sem);
extern void sem_up (struct semaphore *sem);
extern void sem_down (struct semaphore *sem);


#endif /* SEM_H_ */
