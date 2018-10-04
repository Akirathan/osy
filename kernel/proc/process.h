/**
 * @file process.h
 *
 * Process management.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>


/** User process entry point */
#define USER_CODE_START  0x00001000

/** User process stack */
#define USER_STACK_SIZE   THREAD_STACK_SIZE
#define USER_STACK_START  (0x7FFFF000 - USER_STACK_SIZE)


/** Forward declaration */
struct uthread;


/** Process control structure
 *
 */
typedef struct process {
	/** A process can be an item on a list */
	link_t link;
	
	/** Current user stack top */
	void *ustack_top;
	
	/** Process main user thread */
	struct uthread *main_uthread;
	
	/** Process image address */
	void *image;
	
	/** Process image size */
	size_t size;
	
	/** Process return value */
	int retval;
	
	/** List of all owned user threads */
	list_t uthread_list;
	
	/** list of all owned user mutexes */
	list_t umutex_list;
} *process_t;


/* Externals are commented with implementation */
extern int process_create (process_t *processp, void *image, size_t size);
extern void process_set_retval (process_t process, int retval);
extern int process_join (process_t process);


#endif
