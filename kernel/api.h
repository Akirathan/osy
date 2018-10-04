/**
 * @file api.h
 *
 * API for kernel tests
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef API_H_
#define API_H_


#include <include/shared.h>
#include <include/c.h>

#include <mm/falloc.h>
#include <mm/malloc.h>
#include <mm/vmm.h>
#include <adt/atomic.h>
#include <proc/thread.h>
#include <synch/sem.h>
#include <synch/mutex.h>
#include <synch/rmutex.h>
#include <synch/condvar.h>
#include <synch/rwlock.h>
#include <time/timer.h>
#include <lib/print.h>
#include <lib/debug.h>
#include <lib/stdarg.h>
#include <drivers/kbd.h>
#include <drivers/dorder.h>
#include <drivers/disk.h>


/* Run the kernel test */
extern void test_run (void);


#endif
