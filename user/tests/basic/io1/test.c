/***
 * Input/output test #1
 *
 * Change Log:
 * 2003/10/22 ghort   created
 * 2003/11/03 ghort   cleaned up and integrated
 * 2003/11/04 pallas  further cleanups
 * 2003/11/07 pallas  modified to pass shorter buffer to getc
 * 2004/10/25 pallas  updated for new assignment specification
 * 2005/01/11 pallas  updated for user space API
 */

static char * desc =
    "Tests basic input output functions (printf, getc, "
    "and gets).\n";


#include <librt.h>
#include "../../include/defs.h"


#define BUFFER_LENGTH  128
#define POLL_DELAY_MS  1000


int main (void)
{
	int key;
	int len;
	char buf [BUFFER_LENGTH];
	
	printf (desc);
	
	/*
	 * Test blocking getc ().
	 */
	printf ("Testing getc (), write something (<enter> to finish):\n");
	do {
		key = getc ();
		if (key != '\n')
			printf ("<'%c', %d> ", key, key);
	} while (key != '\n');
	printf ("\n");
	
	/*
	 * Test blocking gets ().
	 */
	printf ("Testing gets (), write something (<enter> to finish):\n");
	len = gets (buf, sizeof (buf));
	if (buf [len - 1] == '\n')
		buf [len - 1] = '\0';
	
	printf ("You have written: '%s'\n", buf);
	
	// print the result
	printf ("Test passed...\n");
	
	return 0;
} /* test_run */
