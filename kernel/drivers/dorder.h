/**
 * @file dorder.h
 *
 * Dorder device support.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#ifndef DORDER_H_
#define DORDER_H_

#include <include/shared.h>


#define DORDER_ADDRESS           (ADDR_IN_KSEG1 (DEVICE_DORDER_ADDR))
#define DORDER_DEASSERT_ADDRESS  (ADDR_IN_KSEG1 (DEVICE_DORDER_ADDR + 4))


/** Signal message
 *
 */
#define DORDER_MSG_SIGNAL  0x0000CAFE


/** Get the ID of the current CPU
 *
 * @return Identification number of current CPU
 *         (range 0 - 31).
 *
 */
static inline uint32_t cpuid (void)
{
	return (*(volatile uint32_t *) DORDER_ADDRESS);
}


/** Assert dorder interrupt
 *
 * @param cpuid CPU identification number to assert the interrupt
 *              for (range 0 - 31).
 *
 */
static inline void dorder_assert (const uint32_t cpuid)
{
	*((volatile uint32_t *) DORDER_ADDRESS) = (1 << cpuid);
}


/** Deassert dorder interrupt pending
 *
 * @param cpuid CPU identification number to deassert the interrupt
 *              pending for (range 0 - 31).
 *
 */
static inline void dorder_deassert (const uint32_t cpuid)
{
	*((volatile uint32_t *) DORDER_DEASSERT_ADDRESS) = (1 << cpuid);
}


/* Externals are commented with implementation */
extern void dorder_handle (void);
extern void dorder_send (const uint32_t cpuid, native_t msg);
extern void dorder_receive (native_t msg);
extern void dorder_wait (void);
extern int dorder_probe (void);


#endif
