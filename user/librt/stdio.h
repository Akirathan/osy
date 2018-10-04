/**
 * @file stdio.h
 *
 * Input/output functions.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef LIBRT_STDIO_H_
#define LIBRT_STDIO_H_


#include <types.h>


/* Externals are commented with implementation */
extern size_t putc (const char c);
extern size_t puts (const char *str);
extern size_t printf (const char *format, ...);
extern char getc (void);
extern ssize_t gets (char *buffer, const size_t size);


#endif
