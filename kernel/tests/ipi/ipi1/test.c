/***
 * Inter-processor interrupts test #1
 *
 * Change Log:
 * 2015/12/10 md created
 */

static char * desc =
    "IPI test #1\n"
    "Tests basic inter-processor interrupt operation using "
    "the dorder device.\n\n";


#include <api.h>
#include "../../include/defs.h"
#include "../../include/tst_rand.h"


#define POLL_DELAY_MS   1000
#define SEND_DELAY_MS   250
#define SEND_DELAY_RND  250

#define SEND_COUNT  32


static void *thread_send (void *data)
{
	for (unsigned int cnt = 0; cnt < SEND_COUNT; cnt++) {
		dorder_send (cpuid (), cnt);
		
		/* Sleep with a slight randomization */
		thread_usleep ((SEND_DELAY_MS +
		    (tst_rand () % SEND_DELAY_RND)) * 1000);
	}
	
	dorder_send (cpuid (), DORDER_MSG_SIGNAL);
	
	return NULL;
}


void test_run (void)
{
	printk (desc);
	
	/*
	 * Test blocking dorder_wait ().
	 */
	printk ("Testing dorder_wait ():\n");
	
	/*
	 * Create the sending thread.
	 */
	thread_t thread0 = robust_thread_create (thread_send, NULL, 0);
	
	dorder_wait ();
	printk ("\n");
	
	/*
	 * Join the sending thread.
	 */
	if (thread_join (thread0, NULL) != EOK) {
		printk ("Error joining the sending thread.\n");
		return;
	}
	
	/*
	 * Test non-blocking dorder_probe ().
	 */
	printk ("Testing dorder_probe ():\n");
	
	/*
	 * Create the sending thread.
	 */
	thread_t thread1 = robust_thread_create (thread_send, NULL, 0);
	
	int flag;
	do {
		flag = dorder_probe ();
		if (!flag) {
			/*
			 * Wait a while.
			 */
			thread_usleep (POLL_DELAY_MS * 1000);
			printk (".");
			
		}
	} while (!flag);
	printk ("\n");
	
	/*
	 * Join the sending thread.
	 */
	if (thread_join (thread1, NULL) != EOK) {
		printk ("Error joining the sending thread.\n");
		return;
	}
	
	/* This statement must be reached */
	printk ("Test passed ...\n");
}
