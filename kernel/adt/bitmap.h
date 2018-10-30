/**
 * @file bitmap.h
 *
 * Bit map.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2015
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef BITMAP_H_
#define BITMAP_H_


#include <include/c.h>


#define BITMAP_ELEMENT_BITS  8
#define BITMAP_REMAINER      7


/** Bitmap
 *
 */
typedef struct {
	size_t elements;  /* size of bitmap */
	uint8_t *bits;
} bitmap_t;


static inline void bitmap_set (bitmap_t *bitmap, size_t element,
    unsigned int value)
{
	if (element >= bitmap->elements)
		return;
	
	size_t byte = element / BITMAP_ELEMENT_BITS;
	uint8_t mask = 1 << (element & BITMAP_REMAINER);
	
	if (value) {
		bitmap->bits[byte] |= mask;
	} else {
		bitmap->bits[byte] &= ~mask;
	}
}


static inline unsigned int bitmap_get (bitmap_t *bitmap, size_t element)
{
	if (element >= bitmap->elements)
		return 0;
	
	size_t byte = element / BITMAP_ELEMENT_BITS;
	uint8_t mask = 1 << (element & BITMAP_REMAINER);
	
	return !!((bitmap->bits)[byte] & mask);
}


/* Externals are commented with implementation. */
extern size_t bitmap_size (size_t elements);
extern void bitmap_init (bitmap_t *bitmap, size_t elements, void *storage);
extern int bitmap_check_range (bitmap_t *bitmap, size_t start, size_t count);
extern void bitmap_set_range (bitmap_t *bitmap, size_t start, size_t count);
extern void bitmap_clear_range (bitmap_t *bitmap, size_t start, size_t count);
extern int bitmap_allocate_range (bitmap_t *bitmap, size_t count, size_t base,
    size_t constraint, size_t *index);
extern int bitmap_allocate_range_user (bitmap_t *bitmap, size_t count,
    size_t base, size_t user, size_t constraint, size_t *index);


#endif
