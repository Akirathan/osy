/**
 * @file sys_mutex.h
 *
 * User space mutexes.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef SYS_MUTEX_H_
#define SYS_MUTEX_H_

#include <include/shared.h>
#include <include/c.h>
#include <synch/mutex.h>


/** User mutex control structure
 *
 */
typedef struct umutex {
	/** An user mutex can be an item on a list */
	link_t link;
	
	/** Kernel mutex */
	struct mutex mtx;
} *umutex_t;


/* Externals are commented with implementation */
extern unative_t sys_mutex_init (unative_t *mid);
extern unative_t sys_mutex_lock (unative_t mid);
extern unative_t sys_mutex_unlock (unative_t mid);
extern unative_t sys_mutex_destroy (unative_t mid);


#endif
