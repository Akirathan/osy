/**
 * @file syscall.h
 *
 * System call interface.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef SYSCALL_H_
#define SYSCALL_H_


#include <include/shared.h>
#include <include/c.h>


/** Syscall types
 *
 */
typedef enum {
	SYS_EXIT = 0,
	SYS_PUTC,
	SYS_PUTSTR,
	SYS_GETC,
	SYS_VMA_MAP,
	SYS_VMA_UNMAP,
	SYS_THREAD_CREATE,
	SYS_THREAD_SELF,
	SYS_THREAD_USLEEP,
	SYS_THREAD_JOIN,
	SYS_THREAD_FINISH,
	SYS_MUTEX_INIT,
	SYS_MUTEX_LOCK,
	SYS_MUTEX_UNLOCK,
	SYS_MUTEX_DESTROY,
	SYSCALL_COUNT
} syscall_t;


/** Syscall handler */
typedef unative_t (* syscall_handler) (unative_t p1, unative_t p2, unative_t p3,
    unative_t p4);


/* Externals are commented with implementation */
extern void syscall (context_t *registers);


#endif
