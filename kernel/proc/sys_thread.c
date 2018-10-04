/**
 * @file sys_thread.c
 *
 * User space thread management.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <mm/malloc.h>
#include <mm/vmm.h>
#include <proc/thread.h>
#include <proc/process.h>
#include <lib/string.h>

#include <proc/sys_thread.h>


/** User thread stub
 *
 * Set up the execution of an user space thread.
 * The primary goal is setting up the thread stack
 * and ultimatively switching the CPU to user mode.
 *
 * @param data User thread control structure.
 *
 * @return Thread return value (unreachable).
 *
 */
static void *uthread_stub (void *data)
{
	struct uthread *uthread = (struct uthread *) data;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	/*
	 * Create virtual memory area for the user
	 * space stack.
	 */
	uthread->process->stack_top -=
	    ALIGN_UP (USER_STACK_SIZE, PAGE_SIZE << 1);
	void *base = uthread->process->stack_top;
	
	conditionally_enable_interrupts (state);
	
	size_t size = ALIGN_UP (USER_STACK_SIZE, PAGE_SIZE);
	int rc = vma_map (&base, size, VF_AT_KUSEG | VF_VA_USER);
	if (rc != EOK)
		return NULL;
	
	/*
	 * Set the process as the owner of the current thread.
	 */
	thread_set_process (uthread->process, uthread);
	
	/*
	 * Set EXL = 1 (forces exception level and disables interrupts)
	 * Set KSU = 10 (forces user mode upon exception return)
	 * Set IE = 1 (enables interrupts upon exception return)
	 */
	unative_t status = read_cp0_status ();
	
	status |= CP0_STATUS_EXL_MASK;
	status &= ~CP0_STATUS_KSU_MASK;
	status |= CP0_STATUS_KSU_UM;
	status |= CP0_STATUS_IE_MASK;
	
	write_cp0_status (status);
	
	/*
	 * Set EPC to the user space process entry point.
	 * This is safe since the interrupts are currently
	 * disabled.
	 */
	write_cp0_epc (uthread->entry);
	
	/*
	 * Switch to user space.
	 */
	cpu_uspace_jump (base + USER_STACK_SIZE, uthread->data,
	    uthread->user_data);
	
	/* Unreachable */
	return NULL;
}


/** Create a new user space thread
 *
 * Create a new user space thread by creating its control
 * structure and adding it to the list of user space threads
 * of the current process.
 *
 * @param tid       Place to store the created thread ID (in user memory).
 * @param entry     Thread entry function (in user memory).
 * @param data      Data for the entry function.
 * @param user_data User data for the entry function.
 *
 * @return EOK if the process creation was successful.
 * @return Error code otherwise.
 *
 */
unative_t sys_thread_create (uintptr_t *tid, void *entry, void *data,
    void *user_data)
{
	/*
	 * Check whether it is safe to access the output
	 * argument.
	 */
	
	// TODO: Make sure to check that the tid pointer points to a safely
	//       mapped user memory. The check should be done using
	//       vma_check_user().
	
	/*
	 * Allocate the user thread control structure first.
	 */
	struct uthread *uthread = (struct uthread *) malloc (sizeof (struct uthread));
	if (! uthread)
		return ENOMEM;
	
	uthread->process = thread_get_process ();
	uthread->entry = entry;
	uthread->data = data;
	uthread->user_data = user_data;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	plist_append (&uthread->process->thread_list, &uthread->link);
	conditionally_enable_interrupts (state);
	
	/*
	 * Create the kernel thread.
	 */
	int rc = thread_create (&uthread->thread, uthread_stub, uthread, 0);
	if (rc != EOK) {
		/* Disable interrupts while accessing shared structures. */
		state = query_and_disable_interrupts ();
		plist_append (&uthread->process->thread_list, &uthread->link);
		conditionally_enable_interrupts (state);
		
		free (uthread);
		return rc;
	}
	
	(* tid) = (uintptr_t) uthread;
	return EOK;
}


/** Return the currently executing thread ID
 *
 * @return Currently executing thread ID.
 *
 */
unative_t sys_thread_self (void)
{
	return (uintptr_t) thread_get_uthread ();
}


/** Suspend the current thread for given number of microseconds
 *
 * Suspend the currently executing thread for the given
 * number of microseconds.
 *
 * @param usec Number of microseconds to suspend the thread.
 *
 */
unative_t sys_thread_usleep (const unsigned int usec)
{
	// TODO: Call the appropriate kernel function here
	//       (this is not hard).
	
	/* Return value ignored */
	return EOK;
}


/** Join a thread.
 *
 * Suspend the current thread until another thread exits.
 *
 * @param tid           Thread ID to wait for.
 * @param thread_retval Storage for joining thread return value (in user
 *                      memory. If set to NULL, the return value is discarded.
 *
 * @return EOK if the thread was joined successfully
 * @return EINVAL if the thread is invalid, already being joined
 *         by other thread or if a thread calls join on itself.
 *
 */
unative_t sys_thread_join (uintptr_t tid, void **thread_retval)
{
	/*
	 * Check whether it is safe to access the output
	 * argument. A NULL pointer is safe because thread_join()
	 * won't store the return value into a NULL pointer.
	 */
	
	// TODO: Make sure to check that the tid pointer points to a safely
	//       mapped user memory. The check should be done using
	//       vma_check_user().
	//
	// Note that the NULL value is OK (even if the zero page is unmapped
	
	/*
	 * Check whether the requested thread to join
	 * is a valid thread of the current process.
	 */
	
	process_t process = thread_get_process ();
	uthread_t uthread = NULL;
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	// TODO: Convert the thread ID to the actual user thread
	//       control structure (uthread_t).
	//
	// Note that the process control structure (process_t) contains
	// a list of valid user threads.
	(void) process;
	
	conditionally_enable_interrupts (state);
	
	/* Invalid thread ID */
	if (uthread == NULL)
		return EINVAL;
	
	// TODO: Call the appropriate kernel function here
	//       (this is not hard).
	return EOK;
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
unative_t sys_thread_finish (void *thread_retval)
{
	// TODO: Call the appropriate kernel function here
	//       (this is not hard).
	
	/* Unreachable */
	return EOK;
}
