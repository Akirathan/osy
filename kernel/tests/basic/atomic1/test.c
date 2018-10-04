/**
 * Atomic test #1
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

/***
 * Change Log:
 * 08/10/06 md  initial version
 */

static const char *desc =
    "Atomic test #1\n\n"
    "Tests the basic functionality of atomic variables.\n"
    "Creates an atomic variable and tries to alter the variable.\n"
    "Only the trivial single-threaded variant is tested.\n\n";


#include <api.h>


#define RETURN(msg) \
	{ \
		puts (msg "\nTest failed...\n"); \
		return; \
	}


void test_run (void)
{
	puts (desc);
	
	atomic_t a;
	atomic_set (&a, 42);
	
	if (atomic_get (&a) != 42)
		RETURN ("Failed atomic_set/atomic_get");
	
	if (atomic_add (&a, 10) != 52)
		RETURN ("Failed atomic_add");
	
	if (atomic_get (&a) != 52)
		RETURN ("Failed atomic_get after atomic_add");
	
	if (atomic_sub (&a, 11) != 41)
		RETURN ("Failed atomic_sub");
	
	if (atomic_get (&a) != 41)
		RETURN ("Failed atomic_get after atomic_sub");
	
	puts ("Test passed...\n");
}
