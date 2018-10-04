/**
 * @file thread.c
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

#include <syscall.h>

#include <thread.h>


/** Thread stub
 *
 * Stub function for running threads.
 *
 * @param thread_start Thread body.
 * @param data         Thread data.
 *
 */
static void thread_stub (void *(*thread_start)(void *), void *data)
{
	void *retval = thread_start (data);
	thread_finish (retval);
}


/** Create a new thread
 *
 * Create a new thread and schedule it for execution.
 *
 * @param thread_ptr   Pointer to thread_t holder.
 * @param thread_start Thread entry function.
 * @param data         Data for the entry function.
 *
 * @return EOK if the thread creation succeeded, error code otherwise.
 *
 */
int thread_create (thread_t *thread_ptr, void *(*thread_start)(void *),
    void *data)
{
	// TODO: Call the SYS_THREAD_CREATE syscall
	//
	// Note that the second argument is not thread_start, but
	// the thread stub routine (thread_stub()). The user routine
	// pointed by thread_start is executed via thread_stub().
	(void) thread_stub;
	return EOK;
}


/** Return the currently executing thread ID
 *
 * @return Currently executing thread ID.
 *
 */
thread_t thread_self (void)
{
	// TODO: Call the SYS_THREAD_SELF syscall
	return 0;
}


/** Suspend the current thread for given number of seconds
 *
 * Suspend the currently executing thread for the given
 * number of seconds.
 *
 * @param sec Number of seconds to suspend the thread.
 *
 */
void thread_sleep (const unsigned int sec)
{
	for (unsigned int cnt = 0; cnt < sec; cnt++)
		thread_usleep (1000000);
}


/** Suspend the current thread for given number of microseconds
 *
 * Suspend the currently executing thread for the given
 * number of microseconds.
 *
 * @param usec Number of microseconds to suspend the thread.
 *
 */
void thread_usleep (const unsigned int usec)
{
	// TODO: Call the SYS_THREAD_USLEEP syscall (simply pass the argument)
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
int thread_join (thread_t thr, void **thread_retval)
{
	// TODO: Call the SYS_THREAD_JOIN syscall (simply pass the arguments)
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
void thread_finish (void *thread_retval)
{
	// TODO: Call the SYS_THREAD_FINISH syscall (simply pass the arguments)
}


/** Terminate the process
 *
 * Terminate the current process and set its return
 * value.
 *
 * @param retval Process return value.
 *
 */
void exit (int retval)
{
	SYSCALL1 (SYS_EXIT, (unative_t) retval);
}
