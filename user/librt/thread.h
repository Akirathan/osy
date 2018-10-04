/**
 * @file thread.h
 *
 * User space threading support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */


#ifndef LIBRT_THREAD_H_
#define LIBRT_THREAD_H_


#include <types.h>


/** User space thread ID */
typedef unative_t thread_t;


/* Externals are commented with implementation */
extern int thread_create (thread_t *thread_ptr, void *(*thread_start)(void *),
    void *data);
extern thread_t thread_self (void);
extern void thread_sleep (const unsigned int sec);
extern void thread_usleep (const unsigned int usec);
extern int thread_join (thread_t thr, void **thread_retval);
extern void thread_finish (void *thread_retval);
extern void exit (int retval);


#endif
