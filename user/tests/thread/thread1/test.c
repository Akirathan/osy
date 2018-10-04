/***
 * Thread test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 pallas  cleaned up and integrated
 * 2003/11/07 pallas  converted to robust_thread_create
 * 2004/10/25 pallas  updated for new assignment specification
 * 2005/01/13 pallas  updated for user space API
 * 2005/12/19 pallas  brought up-to-date with user space API
 */

static char * desc =
    "Basic test of thread facilities such as create, "
    "sleep and join.\n";


#include <librt.h>
#include "../../include/defs.h"

/*
 * Data for the threads.
 */
#define THREAD_A_DATA  ((void*) 0xcafebabe)
#define THREAD_B_DATA  ((void*) 0xfacebead)
#define THREAD_C_DATA  ((void*) 0xdeadbeef)


static void * thread_proc_a (void * data)
{
	printf ("Thread A: thread=%x data=%p\n",
	    thread_self (), data);
	
	printf ("Thread A: sleeping for 1 second\n");
	thread_sleep (1);
	
	printf ("Thread A: sleeping for 2 seconds\n");
	thread_sleep (2);
	
	printf ("Thread A: sleeping for 3 seconds\n");
	thread_sleep (3);
	
	printf ("Thread A: sleeping for 1 microsecond\n");
	thread_usleep (1);
	
	printf ("Thread A: sleeping for 1000 microseconds\n");
	thread_usleep (1000);
	
	printf ("Thread A: sleeping for 1000000 microseconds\n");
	thread_usleep (1000000);
	
	printf ("Thread A: sleeping again for 1 second\n");
	thread_sleep (1);
	
	printf ("Thread A: exitting\n");
	return NULL;
} /* thread_proc_a */

static void * thread_proc_c (void * data)
{
	int cnt;
	
	printf ("Thread C: thread=%x data=%p\n",
	    thread_self (), data);
	
	for (cnt = 0; cnt < 5; cnt++) {
		printf ("Thread C: 1 second tick\n");
		thread_sleep (1);
	}
	
	printf ("Thread C: exitting\n");
	return NULL;
} /* thread_proc_c */

static void * thread_proc_b (void * data)
{
	thread_t thread_c;
	
	printf ("Thread B: thread=%x data=%p\n",
	    thread_self (), data);
	
	printf ("Thread B: waiting 2 seconds before starting thread C\n");
	thread_sleep (2);
	
	thread_c = robust_thread_create (thread_proc_c, THREAD_C_DATA);
	printf ("Thread B: started thread C, thread=%x data=%p\n",
	    thread_c, THREAD_C_DATA);
	
	printf ("Thread B: joining thread C\n");
	robust_thread_join (thread_c);
	
	printf ("Thread B: exitting\n");
	return NULL;
} /* thread_proc_b */

/*
 * Intermediate function to make robust_* definitions that return pointers work
 */
static void * main_thread (void)
{
	thread_t thread_a;
	thread_t thread_b;
	int thread_a_active;
	int thread_b_active;
	
	/*
	 * Create the threads.
	 */
	thread_a = robust_thread_create (thread_proc_a, THREAD_A_DATA);
	printf ("Started thread A: thread=%x data=%p\n",
	    thread_a, THREAD_A_DATA);
	thread_a_active = 1;
	
	thread_b = robust_thread_create (thread_proc_b, THREAD_B_DATA);
	printf ("Started thread B: thread=%x data=%p\n",
	    thread_b, THREAD_B_DATA);
	thread_b_active = 1;
	
	/*
	 * Try joining the threads.
	 */
	while ((thread_a_active) || (thread_b_active)) {
		int result;
		
		if (thread_a_active) {
			result = thread_join (thread_a, NULL);
			if (result == EOK) {
				// we joined the thread
				thread_a_active = 0;
			}
		}
		
		if (thread_b_active) {
			result = thread_join (thread_b, NULL);
			if (result == EOK) {
				// we joined the thread
				thread_b_active = 0;
			}
		}
		
		printf ("*");
	}
	
	printf ("\n");
	
	return NULL;
}

int main (void)
{
	printf (desc);
	
	char * ret = main_thread();
	
	// print the result
	if (ret == NULL) {
		printf("\nTest passed...\n\n");
		return 0;
	} else {
		printf("\n%s\n\n", ret);
		return 1;
	}
}
