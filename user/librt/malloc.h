/**
 * @file malloc.h
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

#ifndef LIBRT_MALLOC_H_
#define LIBRT_MALLOC_H_


#include <types.h>


/* Externals are commented with implementation */
extern void heap_init (void);
extern void *malloc (const size_t size);
extern void free (const void *ptr);


#endif
