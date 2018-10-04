/***
 * Condition variable test #2
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/04 pallas  more cleanups, dynamically allocated primitives
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Condition variable test #2\n"
    "Creates an array of condition variables and blocks a few\n"
    "threads on each of them. It then signals every variable\n"
    "once and finally broadcasts to every variable once to\n"
    "finish the test.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * Number of threads waiting on a single condition variable.
 */
#define THREADS_WAITING  (TASK_SIZE / 3)

/*
 * The number of condition variables.
 */
#define CONDVAR_COUNT  (TASK_SIZE * 5)

/*
 * The delay before signalling the condition variable after
 * all threads are running.
 */
#define SIGNAL_DELAY  5


/*
 * Number of running threads and associated mutex.
 */
static volatile int threads_running;
static struct mutex threads_running_mtx;

/*
 * Condition variables and associated mutexes.
 */
static struct condvar * condvar_array [CONDVAR_COUNT];
static struct mutex * mutex_array [CONDVAR_COUNT];


static void *
thread_proc (void * data)
{
	int condvar_idx;
	condvar_idx = (int) data;
	
	inc_var_mutex (threads_running, threads_running_mtx);
	
	mutex_lock (mutex_array [condvar_idx]);
	condvar_wait (condvar_array [condvar_idx], mutex_array [condvar_idx]);
	mutex_unlock (mutex_array [condvar_idx]);
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


void test_run (void)
{
	int cnt;
	int tcnt;
	struct mutex * mutex_mem;
	struct condvar * condvar_mem;
	thread_t threads [CONDVAR_COUNT] [THREADS_WAITING];
	
	printk (desc);
	
	// init
	threads_running = 0;
	mutex_init (&threads_running_mtx);
	
	mutex_mem = (struct mutex *) malloc (
	    CONDVAR_COUNT * sizeof (struct mutex));
	condvar_mem = (struct condvar *) malloc (
	    CONDVAR_COUNT * sizeof (struct condvar));
	
	if ((!condvar_mem) || (!mutex_mem)) {
		printk ("Unable to allocate memory for synchronization "
		    "primitives.\n"
		    "Test failed...\n");
		return;
	}
	
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		mutex_init (mutex_mem);
		mutex_array [cnt] = mutex_mem++;
		
		condvar_init (condvar_mem);
		condvar_array [cnt] = condvar_mem++;
	}
	
	mutex_mem = mutex_array [0];
	condvar_mem = condvar_array [0];
	
	
	/*
	 * Start the threads.
	 */
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		for (tcnt = 0; tcnt < THREADS_WAITING; tcnt++) {
			threads [cnt] [tcnt] = robust_thread_create (
			    thread_proc, (void *) cnt, 0);
		}
	}
	
	/*
	 * Wait until all the threads are started, then give them some
	 * time to start waiting on their respective condition variable.
	 */
	cond_wait_mutex_verbose (
	    threads_running < CONDVAR_COUNT * THREADS_WAITING,
	    threads_running_mtx, threads_running);
	
	printk ("Giving threads %d seconds to start waiting: ", SIGNAL_DELAY);
	for (cnt = 0; cnt < SIGNAL_DELAY; cnt++) {
		thread_sleep (1);
		printk (".");
	}
	printk ("\n");
	
	/*
	 * Signal the variables.
	 */
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		condvar_signal (condvar_array [cnt]);
	}
	
	printk ("If the test falls in an infinite loop here, it means\n"
	    "that condvar_signal did not wake up some threads.\n");
	
	/*
	 * Wait until the signalled threads end.
	 */
	cond_wait_mutex_verbose (
	    threads_running > CONDVAR_COUNT * (THREADS_WAITING - 1),
	    threads_running_mtx, threads_running);
	
	/*
	 * Signal all the other threads.
	 */
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		condvar_broadcast (condvar_array [cnt]);
	}
	
	printk ("If the test falls in an infinite loop here, it means\n"
	    "that condvar_broadcast did not wake up some threads.\n");
	
	/*
	 * Wait until all the spawned threads end.
	 */
	cond_wait_mutex_verbose (threads_running > 0, threads_running_mtx,
	    threads_running);
	
	/*
	 * Reap the threads.
	 */
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		for (tcnt = 0; tcnt < THREADS_WAITING; tcnt++)
			robust_thread_join (threads [cnt] [tcnt]);
	}
	
	
	// clean up
	for (cnt = 0; cnt < CONDVAR_COUNT; cnt++) {
		mutex_destroy (mutex_array [cnt]);
		condvar_destroy (condvar_array [cnt]);
	}
	
	free (mutex_mem);
	free (condvar_mem);
	
	mutex_destroy (&threads_running_mtx);
	
	// print the result
	printk ("Test passed...\n");
}
