/**
 * @file thread.h
 *
 * Kernel thread management.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef THREAD_H_
#define THREAD_H_


#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>
#include <synch/sem.h>
#include <time/timer.h>
#include <mm/vmm.h>


/** Thread stack size
 *
 * The size of the thread stack. This should be set liberally,
 * since stack overflow typically has obscure syndroms and
 * is notoriously difficult to debug.
 *
 */
#define THREAD_STACK_SIZE  4096


/** Thread creation flags.
 *
 */
typedef enum {
	TF_NONE = 0,
	TF_NEW_VMM = (1 << 0)
} thread_flags_t;


/** Forward declarations */
struct process;
struct uthread;


/** Thread entry function */
typedef void *(* thread_fn) (void *data);


/** Thread state
 *
 */
typedef enum {
	THREAD_READY,     /**< thread is ready to run, waiting in a queue */
	THREAD_RUNNING,   /**< thread is currently running */
	THREAD_SLEEPING,  /**< thread is sleeping, moved to sleep list */
	THREAD_ZOMBIE     /**< thread has finished, waiting for reaping */
} thread_state_t;


/** Thread control structure
 *
 * The structure represents a kernel thread. Since the thread context
 * is kept on the thread stack, the only thing that the structure
 * contains is a reference to the stack. The structure can also
 * be an item on a list.
 *
 */
typedef struct thread {
	/** A thread can be an item on a list */
	link_t link;
	
	/** Thread stack size */
	size_t stack_size;
	
	/** Thread stack */
	uint8_t *stack_data;
	
	/** Thread entry function */
	thread_fn entry_func;
	
	/** Entry function data */
	void *entry_data;
	
	/** Return value */
	void *retval;
	
	/** Stack top is where the thread context is */
	void *stack_top;
	
	/** Thread state */
	thread_state_t state;
	
	/** Timestamp when the thread was scheduled */
	unative_t scheduled;
	
	/** Other thread sleeping in join */
	struct thread *joiner;
	
	/** Timer for thread sleep */
	struct timer timer;
	
	/** Wait queue link */
	link_t wait_queue_link;
	
	/** Virtual memory map */
	struct vmm *vmm;
	
	/** Owning process */
	struct process *process;
	
	/** User space thread */
	struct uthread *uthread;
} *thread_t;


/** Currently running threads on each CPU */
extern thread_t current_thread[MAX_CPU];


/* Externals are commented with implementation */
extern void threads_init (void);
extern int thread_create (thread_t *threadp, thread_fn entry, void *data,
    const thread_flags_t flags);
extern thread_t thread_get_current (void);
extern unsigned int thread_sleep (const unsigned int sec);
extern unsigned int thread_usleep (const unsigned int usec);
extern void thread_yield (void);
extern void thread_suspend (void);
extern void thread_set_process (struct process *process,
    struct uthread *uthread);
extern struct process *thread_get_process (void);
extern struct uthread *thread_get_uthread (void);
extern void thread_finish (void *retval) __attribute__((noreturn));
extern int thread_wakeup (thread_t thread);
extern int thread_join (thread_t thread, void **thread_retval);
extern void thread_switch (thread_t thread);


#endif
