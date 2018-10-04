/**
 * @file string.c
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

#include <include/shared.h>
#include <include/c.h>

#include <lib/string.h>


/** Move memory block without overlapping.
 *
 * Copy cnt bytes from src address to dst address. The source
 * and destination memory areas cannot overlap.
 *
 * @param dst Destination address to copy to.
 * @param src Source address to copy from.
 * @param cnt Number of bytes to copy.
 *
 * @return Destination address.
 *
 */
void *memcpy(void *dst, const void *src, size_t cnt)
{
	uint8_t *dp = (uint8_t *) dst;
	const uint8_t *sp = (uint8_t *) src;
	
	while (cnt-- != 0)
		*dp++ = *sp++;
	
	return dst;
}


/** Fill memory block with zeros
 *
 * Fill the memory block at the address dst with cnt
 * zero bytes.
 *
 * @param dst Memory block to fill.
 * @param cnt Number of zero bytes to fill.
 *
 */
void bzero (void *dst, size_t cnt)
{
	uint8_t *dp = (uint8_t *) dst;
	
	while (cnt-- != 0)
		*dp++ = 0;
}
