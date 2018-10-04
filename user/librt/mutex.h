/**
 * @file mutex.h
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

#ifndef LIBRT_MUTEX_H_
#define LIBRT_MUTEX_H_


#include <types.h>


/** User space mutex control structure.
 *
 */
struct mutex {
	/** Kernel mutex ID */
	unative_t id;
};


/* Externals are commented with implementation */
extern int mutex_init (struct mutex *mtx);
extern int mutex_destroy (struct mutex *mtx);
extern int mutex_lock (struct mutex *mtx);
extern int mutex_unlock (struct mutex *mtx);


#endif

