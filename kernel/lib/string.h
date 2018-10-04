/**
 * @file string.h
 *
 * String functions.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef STRING_H_
#define STRING_H_


#include <include/shared.h>
#include <include/c.h>


/* Externals are commented with implementation */
extern void *memcpy (void *dst, const void *src, size_t cnt);
extern void bzero (void *dst, size_t cnt);


#endif
