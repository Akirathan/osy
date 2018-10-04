/**
 * @file malloc.c
 *
 * User space memory allocator.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <types.h>

#include <malloc.h>


/** Initialize the heap allocator
 *
 */
void heap_init (void)
{
	// TODO:
	// Add real implementation
}


/** Allocate a memory block
 *
 * @param size The size of the block to allocate.
 *
 * @return The address of the block or NULL when not enough memory.
 *
 */
void *malloc (const size_t size)
{
	// TODO:
	// Add real implementation
	
	return NULL;
}


/** Free a memory block
 *
 * @param addr The address of the block.
 *
 */
void free (const void *addr)
{
	// TODO:
	// Add real implementation
}
