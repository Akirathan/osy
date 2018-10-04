/***
 * Semaphore test #2
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 pallas  cleaned up and integrated
 * 2003/11/04 pallas  made the semaphore array dynamically allocated
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Semaphore test #2\n"
    "Creates and array of semaphores and sequentially decrements\n"
    "(and sleeps on) them in one thread and increments them in "
    "another.\n\n";


#include <api.h>
#include "../../include/defs.h"


/*
 * The number of semaphores to create.
 */
#define SEM_COUNT  (TASK_SIZE * 100)


/*
 * Number of running threads and associated mutex.
 */
static volatile unsigned int threads_running;
static struct mutex threads_running_mtx;

/*
 * Semaphores...
 */
static struct semaphore * sem_array [SEM_COUNT];


static void *
thread_proc_down (void * data)
{
	int sem;
	assert (data == THREAD_MAGIC);
	
	for (sem = 0; sem < SEM_COUNT; sem++) {
		sem_down (sem_array [sem]);
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


static void *
thread_proc_up (void * data)
{
	int sem;
	assert (data == THREAD_MAGIC);
	
	for (sem = 0; sem < SEM_COUNT; sem++) {
		sem_up (sem_array [sem]);
		
		/*
		 * Yield so that we don't increment more than one
		 * semaphore at a time.
		 */
		thread_yield ();
	}
	
	dec_var_mutex (threads_running, threads_running_mtx);
	
	return NULL;
}


void
test_run (void)
{
	int cnt;
	int success;
	thread_t up_thread;
	thread_t dn_thread;
	struct semaphore * sem_mem;
	
	printk (desc);
	
	// init
	success = 1;
	threads_running = 2;
	
	mutex_init (&threads_running_mtx);
	
	sem_mem = (struct semaphore *) malloc (
	    SEM_COUNT * sizeof (struct semaphore));
	if (!sem_mem) {
		printk ("Unable to allocate memory for semaphore array.\n"
		    "Test failed...\n");
		return;
	}
	
	for (cnt = 0; cnt < SEM_COUNT; cnt++) {
		sem_init (sem_mem, 0);
		sem_array [cnt] = sem_mem++;
	}
	
	sem_mem = sem_array [0];
	
	
	/*
	 * Start the threads.
	 */
	dn_thread = robust_thread_create (thread_proc_down, THREAD_MAGIC, 0);
	up_thread = robust_thread_create (thread_proc_up, THREAD_MAGIC, 0);
	
	/*
	 * Wait for all the threads to finish.
	 */
	cond_wait_mutex (threads_running > 0, threads_running_mtx);
	
	/*
	 * Reap the threads.
	 */
	robust_thread_join (dn_thread);
	robust_thread_join (up_thread);
	
	/*
	 * Verify that all the semaphores are at 0.
	 */
	for (cnt = 0; cnt < SEM_COUNT; cnt++) {
		if (sem_get_value (sem_array [cnt]) != 0) {
			success = 0;
		}
	}
	
	
	// clean up
	mutex_destroy (&threads_running_mtx);
	
	for (cnt = 0; cnt < SEM_COUNT; cnt++) {
		sem_destroy (sem_array [cnt]);
	}
	
	free (sem_mem);
	
	// print the result
	if (success) {
		printk ("Test passed...\n");
	} else {
		printk ("At least one of the semaphores has nonzero value.\n"
		    "Test failed...\n");
	}
}
