/***
 * Input/output test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/04 pallas  further cleanups
 * 2003/11/07 pallas  modified to pass shorter buffer to getc
 * 2004/10/25 pallas  updated for new assignment specification
 */

static char * desc =
    "Input/output test #1\n"
    "Tests basic input output functions (printk, getc, "
    "and getc_try).\n\n";


#include <api.h>
#include "../../include/defs.h"


#define POLL_DELAY_MS  1000


void test_run (void)
{
	int key;
	
	printk (desc);
	
	/*
	 * Test blocking getc ().
	 */
	printk ("Testing getc (), write something (<enter> to finish):\n");
	do {
		key = getc ();
		if (key != '\n')
			printk ("<'%c', %d> ", key, key);
	} while (key != '\n');
	printk ("\n");
	
	/*
	 * Test nonblocking getc_try ().
	 */
	printk ("Testing getc_try (), write something (<enter> to finish):\n");
	do {
		key = getc_try ();
		if (key == EAGAIN) {
			/*
			 * Wait a while so that the keyboard buffer
			 * can fill a bit.
			 */
			thread_usleep (POLL_DELAY_MS * 1000);
			printk (".");
			
		} else if (key >= 0) {
			if (key != '\n')
				printk ("<'%c', %d> ", key, key);
			
		} // other cases are ignored
		
	} while (key != '\n');
	printk ("\n");
	
	/* This statement must be reached */
	printk ("Test passed ...\n");
}
