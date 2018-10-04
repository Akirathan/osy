/***
 * User space protection check
 */

#include <librt.h>

#define MAGIC  0xbaddcafe

static void *thread_proc (void *data)
{
	return (void *) MAGIC;
}

int main (void)
{
	printf ("Trying to create a thread and store its pointer in kernel memory...\n");
	int rc = thread_create ((thread_t *) 0x80000000, thread_proc, NULL);
	if (rc == EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Trying to create a thread and store its pointer in unmapped memory...\n");
	rc = thread_create ((thread_t *) 0x40000000, thread_proc, NULL);
	if (rc == EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Creating a regular thread...\n");
	thread_t tid;
	rc = thread_create (&tid, thread_proc, NULL);
	if (rc != EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Trying to join a thread and store its pointer in kernel memory...\n");
	rc = thread_join (tid, (void **) 0x80000000);
	if (rc == EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Trying to join a thread and store its pointer in unmapped memory...\n");
	rc = thread_join (tid, (void **) 0x40000000);
	if (rc == EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Trying to join a thread a non-existent thread...\n");
	rc = thread_join (~tid, NULL);
	if (rc == EOK) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("Regular thread join...\n");
	void *retval;
	rc = thread_join (tid, &retval);
	if ((rc != EOK) || (retval != (void *) MAGIC)) {
		printf ("\nTest failed...\n\n");
		return 1;
	}
	
	printf ("\nTest passed...\n\n");
	return 0;
}
