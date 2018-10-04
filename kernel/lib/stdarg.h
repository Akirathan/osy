/**
 * @file stdarg.h
 *
 * Support for varying number of function arguments.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef STDARG_H_
#define STDARG_H_


typedef __builtin_va_list va_list;

#define va_start(ap, last)  __builtin_va_start (ap, last)
#define va_arg(ap, type)    __builtin_va_arg (ap, type)
#define va_end(ap)          __builtin_va_end (ap)
#define va_copy(dst, src)   __builtin_va_copy (dst, src)


#endif /* STDARG_H_ */
