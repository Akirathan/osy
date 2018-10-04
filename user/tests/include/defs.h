/***
 * Shared defs for all the tests.
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/02 pallas  added conditional wait macros
 * 2003/11/03 pallas  added protected variable addition/substraction macros
 * 2003/11/03 ghort   added verbose variant of cond_wait_<sem|mutex> macros
 * 2003/11/07 pallas  added robust_thread_create and THREAD_MAGIC macros
 * 2004/10/25 pallas  updated robust_thread_create to new API
 * 2004/10/25 pallas  added robust_thread_func and friends
 * 2005/12/19 pallas  brought up-to-date with user space API
 */

#ifndef _DEFS_H_
#define _DEFS_H_

/*
 * Base task size.
 */
#define TASK_SIZE  6

/*
 * Dummy thread parameter.
 */
#define THREAD_MAGIC  ((void *) 0xdeadbeef)


/*
 * Conditional wait on mutex.
 */
#define cond_wait_mutex(cond, mutex) \
	{ \
		mutex_lock (&mutex); \
		while ((cond)) { \
			mutex_unlock (&mutex); \
			thread_sleep (1); \
			mutex_lock (&mutex); \
		}; \
		mutex_unlock (&mutex); \
	}


/*
 * Conditional wait on mutex with verbose output.
 */
#define cond_wait_mutex_verbose(cond, mutex, counter) \
	{ \
		mutex_lock (&mutex); \
		while ((cond)) { \
			printf ("  %d threads ...\n", counter); \
			mutex_unlock (&mutex); \
			thread_sleep (1); \
			mutex_lock (&mutex); \
		}; \
		\
		printf ("  %d threads ...\n", counter); \
		mutex_unlock (&mutex); \
	}


/*
 * Mutex protected addition/increment of a variable.
 */
#define add_var_mutex(val, var, mutex) \
	{ \
		mutex_lock (&mutex); \
		var += val; \
		mutex_unlock (&mutex); \
	}

#define inc_var_mutex(var, mutex) \
	add_var_mutex (1, var, mutex)


/*
 * Mutex protected substraction/decrement of a variable.
 */
#define sub_var_mutex(val, var, mutex) \
	{ \
		mutex_lock (&mutex); \
		var -= val; \
		mutex_unlock (&mutex); \
	}

#define dec_var_mutex(var, mutex) \
	sub_var_mutex (1, var, mutex)


/*
 * Robust thread creation. Checks whether the thread was
 * successfully created.
 */
#define robust_thread_create(func, args) \
	({ \
		int _res; \
		thread_t _thr; \
		_res = thread_create (&_thr, func, args); \
		if (_res != EOK) { \
			printf ("Unable to create " \
			    "thread (%d)\n", _res); \
			return "Test failed"; \
		} \
		_thr; \
	})


/*
 * Generic robustness wrapper for thread API functions.
 */
#define robust_thread_func(thread, func, expv, opname) \
	{ \
		int _res; \
		_res = func ((thread)); \
		if (_res != (expv)) { \
			printf ("Failed to %s thread %x " \
			    "(got %d, expected %d).\n", opname, \
			    thread, _res, expv); \
			return "Test failed"; \
		} \
	}


/*
 * Robust thread join. Checks whether the thread was
 * successfully joined.
 */
#define robust_thread_join(thread) \
	{ \
		int _res; \
		_res = thread_join ((thread), NULL); \
		if (_res != EOK) { \
			printf ("Failed to join thread %x " \
			    "(%d).\n", thread, _res); \
			return "Test failed"; \
		} \
	}


/*
 * Robust thread wake up. Checks whether the thread was
 * successfully woken.
 */
#define robust_thread_wakeup(thread) \
	robust_thread_func (thread, thread_wakeup, EOK, "wake up")


/*
 * Robust thread detach. Checks whether the thread was
 * successfully detached.
 */
#define robust_thread_detach(thread) \
	robust_thread_func (thread, thread_detach, EOK, "detach")


/*
 * Robust thread cancel. Checks whether the thread was
 * successfully cancelled.
 */
#define robust_thread_cancel(thread) \
	robust_thread_func (thread, thread_cancel, EOK, "cancel")


/**
 * TRUE/FALSE
 *
 * Primitive boolean types.
 */
#define FALSE  0
#define TRUE   (!FALSE)

/**
 * NULL
 *
 * Defines a null pointer.
 */
#ifndef NULL
	#define NULL  ((void *) 0)
#endif /* NULL */

/**
 * INT_MAX
 *
 * Defines maximal integer value.
 */
#ifndef INT_MAX
	#define INT_MAX ((int) 0x7fffffff)
#endif /* NULL */

/**
 * sizeof_array
 * @array: array to determine the size of
 *
 * Returns the size of @array in array elements.
 */
#define sizeof_array(array) \
	(sizeof (array) / sizeof ((array) [0]))

#define STRING(arg)      STRING_ARG(arg)
#define STRING_ARG(arg)  #arg

#endif /* _DEFS_H_ */
