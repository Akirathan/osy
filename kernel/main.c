/**
 * @file main.c
 *
 * Kernel mainline.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <mm/malloc.h>
#include <proc/thread.h>
#include <sched/sched.h>
#include <mm/tlb.h>
#include <mm/falloc.h>
#include <mm/malloc.h>
#include <lib/print.h>
#include <drivers/dorder.h>
#include <drivers/disk.h>
#include <example.h>

#include <main.h>


/** Idle thread
 *
 * This thread is being scheduled by the application
 * processors and does nothing (it only spins in
 * an infinite loop).
 *
 * Since Kalisto lacks the implementation of any
 * synchronization primitives, it is hard to do
 * any real job on the application processors.
 * Even the output to the dprinter device would
 * just create a mess.
 *
 */
static void *idle (void *data)
{
	while (true)
		thread_yield ();
	
	return NULL;
}


/** The kernel entry point in C code for bootstrap processor (BSP)
 *
 * This function is called by the assembler code shortly after bootstrap,
 * with disabled interrupts and temporary stack. The function initializes
 * the kernel data structures and only then switches to a standard thread.
 *
 * After the initialization is complete, application processors entry
 * points are allowed to proceed.
 *
 */
void bsp_start (void)
{
	/*
	 * Say hello :-) We write a small message after each
	 * initialization stage to make it easier to see
	 * where things go wrong. Should that happen,
	 * of course.
	 */
	puts ("This is Kalisto " QUOTE_ME(KALISTO_VERSION) ",\n" \
		"built by " QUOTE_ME(BUILT_BY) " at " __TIME__ " " __DATE__ ".\n");
	
	/*
	 * Initialize TLB. We are running in an unmapped
	 * segment, initializing TLB is therefore not
	 * strictly necessary, but it might help
	 * as an example for later assignments.
	 */
	puts ("cpu0: Address translation ... ");
	tlb_init ();
	puts ("OK\n");
	
	/* Initialize heap. */
	puts ("cpu0: Frame allocator ... ");
	frame_init ();
	puts ("OK\n");
	
	/* Initialize heap. */
	puts ("cpu0: Heap allocator ... ");
	heap_init ();
	puts ("OK\n");
	
	/* Threading. */
	puts ("cpu0: Threading ... ");
	threads_init ();
	puts ("OK\n");
	
	/* Scheduler. */
	puts ("cpu0: Scheduler ... ");
	scheduler_init ();
	puts ("OK\n");
	
	/* Timers. */
	puts ("cpu0: Timers ... ");
	int rc = timers_init ();
	if (rc != EOK)
		panic ("Unable to initialize timers.");
	puts ("OK\n");
	
	/* Disk. */
	puts ("cpu0: Disk ... ");
	disk_init ();
	puts ("OK\n");
	
	/* Create an idle thread. */
	thread_t idle_thread;
	rc = thread_create (&idle_thread, idle, NULL, 0);
	if (rc != EOK)
		panic ("Unable to create the idle thread.");
	
	/*
	 * Everything is ready for moving
	 * to a standard thread.
	 */
	thread_t thread;
	rc = thread_create (&thread, example, NULL, 0);
	if (rc != EOK)
		panic ("Unable to create the main thread.");
	
	/*
	 * Allow the first AP to run. It is critical to
	 * do this after finishing all memory
	 * allocations since there is no synchronization
	 * (locking) of the memory management.
	 */
	atomic_add (&cpu_ready, 1);
	
	thread_switch (thread);
	
	/*
	 * The execution should never
	 * reach this point.
	 */
	panic ("Execution returned to bsp_start().");
}


/** The kernel entry point in C code for application processor (AP)
 *
 * This function is called by the assembler code shortly after bootstrap,
 * with disabled interrupts and temporary stack. The function waits until
 * the global kernel structures are initialized by bsp_start () and only
 * then switches to a standard thread.
 *
 */
void ap_start (void)
{
	/* Wait until we are ready to run. */
	while (atomic_get(&cpu_ready) < (native_t) cpuid());
	
	/* Initialize local CPU resources. */
	tlb_init ();
	scheduler_init ();
	
	/* Create an idle thread. */
	thread_t idle_thread;
	thread_create (&idle_thread, idle, NULL, 0);
	
	/* Allow the next AP to run. */
	atomic_add (&cpu_ready, 1);
	
	thread_switch (idle_thread);
	
	/*
	 * The execution should never
	 * reach this point.
	 */
	panic ("Execution returned to ap_start().");
}
