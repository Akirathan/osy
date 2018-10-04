/**
 * @file malloc.h
 *
 * Kernel memory allocator.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef MALLOC_H_
#define MALLOC_H_


#include <include/c.h>


/* Externals are commented with implementation */
extern void heap_init (void);
extern void *malloc (const size_t size);
extern void *safe_malloc (const size_t size);
extern void free (const void *ptr);


#endif
