/**
 * @file syscall.c
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

#include <include/shared.h>
#include <include/c.h>

#include <lib/print.h>
#include <proc/thread.h>
#include <proc/process.h>
#include <proc/sys_thread.h>
#include <synch/sys_mutex.h>
#include <drivers/kbd.h>

#include <exc/syscall.h>


/** Handle the SYS_EXIT system call
 *
 * Handle the SYS_EXIT system call by finishing the current
 * process.
 *
 * @param retval Process return value.
 *
 * @return Unused return value.
 *
 */
static unative_t sys_exit (int retval)
{
	process_t process = thread_get_process ();
	if (process == NULL)
		return EINVAL;
	
	process_set_retval (process, retval);
	thread_finish (NULL);
	
	/* Unreachable */
	return EOK;
}


static unative_t sys_putc (char c)
{
	// TODO
}


static unative_t sys_putstr (const char *str, size_t size)
{
	// TODO:
}


static unative_t sys_getc (void)
{
	// TODO
}


static unative_t sys_vma_map (void **from, const size_t size)
{
	// TODO:
}


static unative_t sys_vma_unmap (void *from)
{
	// TODO
}


/** Syscall table
 *
 */
static syscall_handler syscall_table[] = {
	(syscall_handler) sys_exit,
	(syscall_handler) sys_putc,
	(syscall_handler) sys_putstr,
	(syscall_handler) sys_getc,
	(syscall_handler) sys_vma_map,
	(syscall_handler) sys_vma_unmap,
	(syscall_handler) sys_thread_create,
	(syscall_handler) sys_thread_self,
	(syscall_handler) sys_thread_usleep,
	(syscall_handler) sys_thread_join,
	(syscall_handler) sys_thread_finish,
	(syscall_handler) sys_mutex_init,
	(syscall_handler) sys_mutex_lock,
	(syscall_handler) sys_mutex_unlock,
	(syscall_handler) sys_mutex_destroy
};


/** Handle a system call.
 *
 * The function is called from the exception handler when the exception
 * is identified as being caused by a system call. Interrupts are
 * disabled and the various registers identify what system
 * call is requested.
 *
 * Before returning, the routine adjusts the program counter to
 * point to the instruction following the system call.
 *
 * @param registers The registers at the time of the interrupt.
 *
 */
void syscall (context_t *registers)
{
	// TODO:
	// Check to see if the syscall occured in the branch delay
	// slot. We are not supposed to handle this case.
	
	/*
	 * Make sure we skip the syscall instruction on return.
	 */
	registers->epc += 4;
	
	if (registers->v0 < SYSCALL_COUNT) {
		enable_interrupts ();
		registers->v0 = syscall_table[registers->v0](registers->a0,
		    registers->a1, registers->a2, registers->a3);
		disable_interrupts ();
	} else {
		printk ("Thread %x called undefined syscall %u\n",
		    thread_get_current (), registers->v0);
		thread_finish (NULL);
	}
}
