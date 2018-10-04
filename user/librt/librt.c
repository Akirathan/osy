/**
 * @file librt.c
 *
 * User space run-time library.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <abi.h>
#include <malloc.h>
#include <thread.h>
#include <debug.h>


/** Run-time entry function
 *
 * This function is called directly from the process
 * entry point. Its purpose is to initialize the run-time
 * library, pass control to the user's main() and terminate
 * the process afterwards.
 *
 */
void __main (void)
{
	/* Initialization */
	heap_init ();
	
	/* Call main(), set return value */
	int rc = main ();
	exit (rc);
	
	/* Unreachable */
	assert(false);
}
