/**
 * @file types.h
 *
 * C types definitions file.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef LIBRT_TYPES_H_
#define LIBRT_TYPES_H_


/***************************************************************************\
| Error codes                                                               |
\***************************************************************************/

#define EOK          0       /* Everything's OK */
#define EAGAIN       -11     /* Try again */
#define ENOMEM       -12     /* Out of memory */
#define EINVAL       -22     /* Invalid argument */
#define ENOSYS       -38     /* Function not implemented */
#define ETIMEDOUT    -145    /* Connection timed out */
#define EKILLED      -666    /* Thread was killed */


/***************************************************************************\
| Basics                                                                    |
\***************************************************************************/

/*
 * All right, not everything
 * needs to be commented :-)
 */
#define false  0
#define true   (!false)

/* An unused pointer value. */
#define NULL  0


/** Basic platform types */
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int32_t native_t;
typedef uint32_t unative_t;
typedef uint32_t uintptr_t;
typedef uint32_t off_t;
typedef uint32_t size_t;
typedef int32_t ssize_t;
typedef uint8_t bool;


#endif
