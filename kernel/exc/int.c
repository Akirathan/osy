/**
 * @file int.c
 *
 * Interrupt handling.
 *
 * Kalisto
 *
 * Copyright (c) 2001-2010
 *   Department of Distributed and Dependable Systems
 *   Faculty of Mathematics and Physics
 *   Charles University, Czech Republic
 *
 */

#include <include/shared.h>
#include <include/c.h>

#include <proc/thread.h>
#include <sched/sched.h>
#include <drivers/kbd.h>

#include <exc/int.h>


/** Handle an interrupt
 *
 * The function is called from the exception handler when the exception
 * is identified as being caused by an interrupt request. Interrupts
 * are disabled and the IP bits in the Cause register identify which
 * interrupt requests are active. More than one bit can be set.
 *
 * The handler has to do whatever is necessary to satisfy the interrupt
 * request, otherwise the exception will be generated immediately after
 * interrupts are enabled.
 *
 * @param registers The registers at the time of the interrupt.
 *
 */
void interrupt (context_t *registers)
{
	/* Get the IP bits from the Cause register. */
	int cause = registers->cause;
	
	if (cause & CP0_CAUSE_IP4_MASK) {
		/*
		 * IP4 is a keyboard interrupt.
		 */
		kbd_handle ();
	}
	
	if (cause & CP0_CAUSE_IP7_MASK) {
		/*
		 * IP7 is a timer interrupt.
		 *
		 * The scheduler handlers timer interrupts until a timer
		 * framework is implemented. Since the scheduler switches
		 * context, it is a good idea to handle the timer interrupt
		 * as the last one, when all other interrupts are satisfied.
		 */
		sched_timer ();
	}
	
	/* All interrupt requests should be handled by now. */
}
