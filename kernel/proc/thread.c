/**
 * @file thread.c
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

#include <include/shared.h>
#include <include/c.h>

#include <adt/list.h>
#include <mm/malloc.h>
#include <sched/sched.h>
#include <drivers/dorder.h>
#include <time/time.h>
#include <lib/print.h>

#include <proc/thread.h>


/** Currently running threads on each CPU */
thread_t current_thread[MAX_CPU];


/** Initialize threads management
 *
 * Set current threads on all CPUs
 * to be NULL.
 *
 */
void threads_init (void)
{
	/* Initialize the state of current threads. */
	for (unsigned int i = 0; i < MAX_CPU; i++)
		current_thread[i] = NULL;
}


/** Thread stub
 *
 * Stub function for running threads.
 *
 * @param thread Thread to run.
 *
 */
static void thread_stub (thread_t thread)
{
	void *retval = thread->entry_func (thread->entry_data);
	thread_finish (retval);
}


/** Create a new thread
 *
 * Create a new thread and schedule it for execution.
 *
 * @param pthread Pointer to thread_t holder.
 * @param entry   Thread entry function.
 * @param data    Data for the entry function.
 * @param flags   Thread creation flags.
 *
 * @return EOK if the thread creation succeeded, error code otherwise.
 *
 */
int thread_create (thread_t *pthread, thread_fn entry, void *data,
    const thread_flags_t flags)
{
	/*
	 * Allocate the control structure first.
	 */
	struct thread *thread = (struct thread *) malloc (sizeof (struct thread));
	if (!thread)
		return ENOMEM;
	
	/*
	 * Allocate the stack.
	 */
	thread->stack_size = THREAD_STACK_SIZE;
	thread->stack_data = (uint8_t *) malloc (THREAD_STACK_SIZE);
	if (!thread->stack_data) {
		free (thread);
		return ENOMEM;
	}
	
	/*
	 * Allocate new virtual memory map if necessary.
	 */
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	
	if ((current == NULL) || ((flags & TF_NEW_VMM) == TF_NEW_VMM)) {
		int rc = vmm_create (&thread->vmm);
		if (rc != EOK) {
			free (thread->stack_data);
			free (thread);
			return rc;
		}
	} else
		thread->vmm = current->vmm;
	
	// TODO:
	// Update the reference count of the virtual memory map.
	
	conditionally_enable_interrupts (state);
	
	/*
	 * Initialize the list membership structure.
	 */
	link_init (&thread->link);
	
	/*
	 * Fill in the thread entry.
	 */
	thread->entry_func = entry;
	thread->entry_data = data;
	
	/*
	 * Empty return value, process and user space thread.
	 */
	thread->retval = NULL;
	thread->process = NULL;
	thread->uthread = NULL;
	
	/*
	 * Stack grows downwards inside the allocated block. Its top
	 * contains the thread context structure, which is restored
	 * on context switch. Of the entire structure, only the
	 * starting address of the thread and some vital
	 * processor registers are set.
	 */
	thread->stack_top = ((char *) thread->stack_data) +
	    THREAD_STACK_SIZE - sizeof (context_t) - ABI_STACK_FRAME;
	
	thread->scheduled = 0;
	thread->state = THREAD_READY;
	thread->joiner = NULL;
	
	link_init (&thread->wait_queue_link);
	
	/*
	 * We use a pointer to the thread context
	 * structure for more readable code.
	 */
	context_t *context = (context_t *) (thread->stack_top);
	
	context->ra = (unative_t) thread_stub;
	context->a0 = (unative_t) thread;
	context->gp = ADDR_IN_KSEG0 (0);
	context->status = CP0_STATUS_IE_MASK | CP0_STATUS_IM_MASK;
	
	sched_insert (thread);
	
	(* pthread) = thread;
	return EOK;
}


/** Give up the processor and force a reschedule
 *
 */
void thread_yield (void)
{
	schedule ();
}


/** Return the currently executing thread
 *
 */
thread_t thread_get_current (void)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	thread_t current = current_thread[cpuid()];
	conditionally_enable_interrupts (state);
	
	return current;
}


/** Suspend the current thread
 *
 * Suspend the currently executing thread indefinitely.
 *
 */
void thread_suspend (void)
{
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	current->state = THREAD_SLEEPING;
	
	sched_remove (current);
	
	schedule ();
	
	conditionally_enable_interrupts (state);
}


/** Wake up a sleeping thread
 *
 * @param thread Thread to wake up.
 *
 */
int thread_wakeup (thread_t thread)
{
	ipl_t state = query_and_disable_interrupts ();
	
	if (thread->state == THREAD_SLEEPING) {
		thread->state = THREAD_READY;
		sched_insert (thread);
	}
	
	conditionally_enable_interrupts (state);
	return EOK;
}


/** Thread timeout handler
 *
 * Wake up the thread that called thread_sleep() or
 * thread_usleep().
 *
 */
static void thread_timeout_handler (struct timer *timer, void *data)
{
	thread_wakeup ((thread_t) data);
}

/** Suspend the current thread for given number of seconds
 *
 * Suspend the currently executing thread for the given
 * number of seconds.
 *
 * @param sec Number of seconds to suspend the thread.
 *
 * @return Number of seconds the thread was actually suspended.
 *
 */
unsigned int thread_sleep (const unsigned int sec)
{
	unsigned int timeout = sec_to_jiffies (sec);
	
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	current->state = THREAD_SLEEPING;
	
	sched_remove (current);
	
	/*
	 * Setup up a timer to wake us up. The timer will be deleted from
	 * the list of timers when it expires and fires the handler.
	 */
	timer_init_jiffies (&current->timer, timeout, thread_timeout_handler,
	    current);
	timer_start (&current->timer);
	
	schedule ();
	
	/*
	 * After wakeup, determine the time remaining to timer expiration.
	 */
	unsigned int remains = current->timer.expires - jiffies;
	
	/*
	 * Destroy the timer. Waits for completion of the timer handler.
	 */
	timer_destroy (&current->timer);
	
	conditionally_enable_interrupts (state);
	return jiffies_to_sec (remains);
}


/** Suspend the current thread for given number of microseconds
 *
 * Suspend the currently executing thread for the given
 * number of microseconds.
 *
 * @param usec Number of microseconds to suspend the thread.
 *
 * @return Number of microseconds the thread was actually suspended.
 *
 */
unsigned int thread_usleep (const unsigned int usec)
{
	unsigned int timeout = usec_to_jiffies (usec);
	
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	current->state = THREAD_SLEEPING;
	
	sched_remove (current);
	
	/*
	 * Setup up a timer to wake us up. The timer will be deleted from
	 * the list of timers when it expires and fires the handler.
	 */
	timer_init_jiffies (&current->timer, timeout, thread_timeout_handler,
	    current);
	timer_start (&current->timer);
	
	schedule ();
	
	/*
	 * After wakeup, determine the time remaining to timer expiration.
	 */
	unsigned int remains = current->timer.expires - jiffies;
	
	/*
	 * Destroy the timer. Waits for completion of the timer handler.
	 */
	timer_destroy (&current->timer);
	
	conditionally_enable_interrupts (state);
	return jiffies_to_usec (remains);
}


/** Verify that a thread can be joined
 *
 * Verify that the current thread can call join on a given thread.
 * Must be called with interrupts disabled so that the thread flags
 * and joiner attributes do not change.
 *
 * @param thread Thread to check.
 *
 */
static bool thread_can_join (struct thread *thread)
{
	ipl_t state = query_and_disable_interrupts ();
	thread_t current = current_thread[cpuid()];
	conditionally_enable_interrupts (state);
	
	/*
	 * Verify that the thread can be joined, which means that it
	 * is not the current thread and it is not being already joined
	 * by somebody else.
	 */
	if (thread == current)
		return false;
	
	if (thread->joiner != NULL)
		return false;
	
	return true;
}


/** Set controlling process and user space thread
 *
 * Set the controlling process and user space thread
 * of the current thread.
 *
 * @param process Controlling process of the current thread.
 * @param uthread User space thread of the current thread.
 *
 */
void thread_set_process (struct process *process, struct uthread *uthread)
{
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	current->process = process;
	current->uthread = uthread;
	
	conditionally_enable_interrupts (state);
}


/** Get controlling process
 *
 * Get the controlling process of the current thread.
 *
 * @return Controlling process of the current thread.
 *
 */
struct process *thread_get_process (void)
{
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	struct process *process = current->process;
	
	conditionally_enable_interrupts (state);
	
	return process;
}


/** Get user space thread
 *
 * Get the user space thread of the current thread.
 *
 * @return User space thread of the current thread.
 *
 */
struct uthread *thread_get_uthread (void)
{
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	struct uthread *uthread = current->uthread;
	
	conditionally_enable_interrupts (state);
	
	return uthread;
}


/** Finish the current thread
 *
 * Prepare the current thread for termination, finish
 * its user code execution. Set the thread return value
 * and wake up any joining thread.
 *
 * @param retval Thread return value.
 *
 */
void thread_finish (void *retval)
{
	query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	
	current->state = THREAD_ZOMBIE;
	
	sched_remove (current);
	
	/* Store the return value */
	current->retval = retval;
	
	/*
	 * If there is a thread calling join on the exitting thread, wake
	 * it up and let it know to clean up the mess.
	 */
	if (current->joiner != NULL)
		thread_wakeup (current->joiner);
	
	/*
	 * Give up the processor now.
	 */
	schedule ();
	
	/* Unreachable */
	panic ("Zombie thread returned from eternity.\n");
}


/** Clean up thread
 *
 * Clean up and release the thread control structure.
 * Must be called with interrupts disabled so that the kernel heap
 * is in a consistent state.
 *
 * @param thread Thread to clean up.
 *
 */
static void thread_destroy (thread_t thread)
{
	// TODO:
	// Update the reference count of the virtual memory map
	// and dispose of an unused virtual memory map if the
	// reference count drops to zero.
	
	free (thread->stack_data);
	free (thread);
}


/** Join a thread.
 *
 * Suspend the current thread until another thread exits.
 *
 * @param thread        Thread to wait for.
 * @param thread_retval Storage for joining thread return value.
 *                      If set to NULL, the return value is discarded.
 *
 * @return EOK if the thread was joined successfully
 * @return EINVAL if the thread is invalid, already being joined
 *         by other thread or if a thread calls join on itself.
 *
 */
int thread_join (thread_t thread, void **thread_retval)
{
	ipl_t status = query_and_disable_interrupts ();
	thread_t current = current_thread[cpuid()];
	
	/*
	 * Verify thread identity and that it can be joined.
	 */
	 if (!thread_can_join (thread))
		goto invalid;
	
	/*
	 * If the thread is not a zombie, wait for it to become one.
	 * The only thing that can wake us up is the thread exitting.
	 */
	if (thread->state != THREAD_ZOMBIE) {
		thread->joiner = current;
		thread_suspend ();
	}
	
	if (thread_retval != NULL)
		(* thread_retval) = thread->retval;
	
	/*
	 * When the thread dies, enable interrupts and reap its remains.
	 */
	thread_destroy (thread);
	conditionally_enable_interrupts (status);
	
	return EOK;
	
invalid:
	conditionally_enable_interrupts (status);
	return EINVAL;
}


/** Start executing given thread
 *
 * The function saves the context of the current thread and
 * restores the context of the thread given as argument.
 *
 * @param thread Thread to start executing.
 *
 */
void thread_switch (thread_t thread)
{
	/*
	 * The only thing to do besides switching the processor context
	 * is to maintain the pointer to the current thread and thread
	 * states. We disable interrupts to do that so that the pointer
	 * to the current thread is always in sync with the current thread.
	 */
	
	ipl_t state = query_and_disable_interrupts ();
	
	thread_t current = current_thread[cpuid()];
	
	if ((current != NULL) && (current->state == THREAD_RUNNING))
		current->state = THREAD_READY;
	
	current_thread[cpuid()] = thread;
	thread->state = THREAD_RUNNING;
	
	/*
	 * One special case to consider here is when switching context
	 * for the very first time. At that time, we are running without
	 * a thread structure and with a temporary stack, and therefore
	 * we do not care about the current stack top that gets saved.
	 */
	
	if (current == NULL) {
		void *dummy_stack_top;
		cpu_switch_context (&dummy_stack_top, &thread->stack_top,
		    thread->vmm->asid);
	} else
		cpu_switch_context (&current->stack_top, &thread->stack_top,
		    thread->vmm->asid);
	
	/*
	 * This is subtle. The cpu_switch_context function will execute
	 * another thread, which might switch to yet another thread,
	 * and so on, eventually switching back to this thread.
	 * At that time, it will look like we have just returned
	 * here from the cpu_switch_context function.
	 */
	
	conditionally_enable_interrupts (state);
}
