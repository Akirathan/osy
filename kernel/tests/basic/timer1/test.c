/***
 * Timer test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   fixed starting of the 2nd timer
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Timer test #1\n"
    "Tests the timer infrastructure.\n\n";


#include <api.h>


/*
 * The duration of the timer test in seconds.
 */
#define TEST_DURATION  6

/*
 * The timeout to initialize the timers with, in milliseconds.
 */
#define TIMER_TIMEOUT  500

/*
 * Magic value passed to the timer handler.
 */
#define TIMER_MAGIC  ((void *) 0xdeadbeef)


/*
 * Global termination flag.
 */
static volatile int finish_flag;


static void
timer_proc1 (struct timer * timer, void * data)
{
	assert (data == TIMER_MAGIC);
	
	putc ('1');
	timer_start (timer);
}

static void
timer_proc2 (struct timer * timer, void * data)
{
	assert (data == TIMER_MAGIC);
	
	putc ('2');
	finish_flag = 1;
}


void
test_run (void)
{
	struct timer tmr1;
	struct timer tmr2;
	
	printk (desc);
	
	// init
	finish_flag = 0;
	
	timer_init_jiffies (&tmr1, TIMER_TIMEOUT, timer_proc1, TIMER_MAGIC);
	timer_init_jiffies (&tmr2, TIMER_TIMEOUT, timer_proc2, TIMER_MAGIC);
	
	
	/*
	 * Test the first timer.
	 */
	printk ("Testing timer #1 (%p):\n", &tmr1);
	timer_start (&tmr1);
	thread_sleep (TEST_DURATION);
	
	timer_destroy (&tmr1);
	printk ("\n");
	
	/*
	 * Test the second timer.
	 */
	printk ("Testing timer #2 (%p):\n", &tmr2);
	
	timer_start (&tmr2);
	while (!finish_flag) {
		thread_sleep (1);
	}
	
	timer_destroy (&tmr2);
	printk ("\n");
	
	
	// print the result
	printk ("Test passed...\n");
}
