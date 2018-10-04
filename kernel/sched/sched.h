/**
 * @file sched.h
 *
 * Round robin kernel thread scheduler.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef SCHED_H_
#define SCHED_H_


#include <adt/atomic.h>
#include <proc/thread.h>


/* Externals are commented with implementation */
ATOMIC_EXTERN(cpu_ready);

extern volatile unsigned int jiffies;


extern void scheduler_init (void);
extern void sched_insert (thread_t thread);
extern void sched_remove (thread_t thread);
extern void sched_timer (void);
extern void schedule (void);


#endif
