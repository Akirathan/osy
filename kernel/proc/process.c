/**
 * @file process.c
 *
 * Process management.
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
#include <proc/sys_thread.h>
#include <lib/string.h>

#include <proc/process.h>


/** Process main thread stub
 *
 * Set up the execution of the process in user space.
 * This includes creating the code/static data and stack
 * virtual memory areas, copying the process binary image
 * and ultimatively switching the CPU to user mode.
 *
 * @param data Process control structure.
 *
 * @return Thread return value (unreachable).
 *
 */
static void *process_stub (void *data)
{
	struct process *process = (struct process *) data;
	
	/*
	 * Create virtual memory area for code
	 * and static data.
	 */
	void *base = (void *) ALIGN_DOWN (USER_CODE_START, PAGE_SIZE);
	size_t size = ALIGN_UP (process->size, PAGE_SIZE);
	int rc = vma_map (&base, size, VF_AT_KUSEG | VF_VA_USER);
	if (rc != EOK)
		return NULL;
	
	/*
	 * Copy the process image to the virtual memory
	 * area previously created.
	 */
	memcpy (base, process->image, process->size);
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	/*
	 * Create virtual memory area for the user
	 * space stack.
	 */
	process->ustack_top = (void *)
	    ALIGN_DOWN (USER_STACK_START, PAGE_SIZE);
	base = process->ustack_top;
	
	conditionally_enable_interrupts (state);
	
	size = ALIGN_UP (USER_STACK_SIZE, PAGE_SIZE);
	rc = vma_map (&base, size, VF_AT_KUSEG | VF_VA_USER);
	if (rc != EOK)
		return NULL;
	
	/*
	 * Set the process as the owner of the current thread.
	 */
	thread_set_process (process, process->main_uthread);
	
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
	write_cp0_epc (USER_CODE_START);
	
	/*
	 * Switch to user space.
	 */
	cpu_uspace_jump (base + USER_STACK_SIZE, NULL, NULL);
	
	/* Unreachable */
	return NULL;
}


/** Create a new process
 *
 * Create a new process by creating its control structure
 * and its main thread.
 *
 * @param pprocess Place to store the created process control
 *                 structure.
 * @param image    Virtual address of the process binary image.
 * @param size     Size of the process binary image.
 *
 * @return EOK if the process creation was successful.
 * @return Error code otherwise.
 *
 */
int process_create (process_t *pprocess, void *image, size_t size)
{
	/*
	 * Allocate the control structure first.
	 */
	struct process *process = (struct process *) malloc (sizeof (struct process));
	if (!process)
		return ENOMEM;
	
	/*
	 * Allocate the user thread control structure first.
	 */
	struct uthread *uthread = (struct uthread *) malloc (sizeof (struct uthread));
	if (!uthread) {
		free (process);
		return ENOMEM;
	}
	
	process->main_uthread = uthread;
	process->image = image;
	process->size = size;
	list_init (&process->uthread_list);
	list_init (&process->umutex_list);
	
	uthread->process = process;
	uthread->entry = (void *) USER_CODE_START;
	uthread->data = NULL;
	uthread->user_data = NULL;
	
	list_append (&process->uthread_list, &uthread->link);
	
	/*
	 * Create the main thread.
	 */
	int rc = thread_create (&uthread->thread, process_stub, process,
	    TF_NEW_VMM);
	if (rc != EOK) {
		list_remove (&uthread->link);
		
		free (uthread);
		free (process);
		return rc;
	}
	
	(* pprocess) = process;
	return EOK;
}


/** Set process return value.
 *
 * Set the return value of a process.
 *
 * @param process Process to modify.
 * @param retval  Return value to set.
 *
 */
void process_set_retval (process_t process, int retval)
{
	ipl_t state = query_and_disable_interrupts ();
	process->retval = retval;
	conditionally_enable_interrupts (state);
}


/** Join a process.
 *
 * Suspend the current thread until the specified process
 * (specifically its main thread) exits.
 *
 * @param process Process to wait for.
 *
 * @return EOK if the process was joined successfully
 * @return EINVAL if the process is invalid, already being joined
 *         by other thread or if a thread calls join on itself.
 *
 */
int process_join (process_t process)
{
	ipl_t state = query_and_disable_interrupts ();
	thread_t thread = process->main_uthread->thread;
	conditionally_enable_interrupts (state);
	
	return thread_join (thread, NULL);
}
