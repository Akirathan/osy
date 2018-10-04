/**
 * @file sys_thread.h
 *
 * User space thread management.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef SYS_THREAD_H_
#define SYS_THREAD_H_

#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>
#include <proc/thread.h>


/** Forward declaration */
struct process;


/** User thread control structure
 *
 */
typedef struct uthread {
	/** An user thread can be an item on a list */
	link_t link;
	
	/** Kernel thread */
	thread_t thread;
	
	/** Process */
	struct process *process;
	
	/** User entry point */
	void *entry;
	
	/** Thread data */
	void *data;
	
	/** Thread user data */
	void *user_data;
} *uthread_t;


/* Externals are commented with implementation */
extern unative_t sys_thread_create (unative_t *tid, void *entry, void *data,
    void *user_data);
extern unative_t sys_thread_self (void);
extern unative_t sys_thread_usleep (const unsigned int usec);
extern unative_t sys_thread_join (unative_t tid, void **thread_retval);
extern unative_t sys_thread_finish (void *thread_retval);


#endif
