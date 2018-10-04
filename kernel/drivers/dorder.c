/**
 * @file dorder.c
 *
 * Dorder device support.
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

#include <adt/atomic.h>
#include <adt/list.h>
#include <proc/thread.h>

#include <drivers/dorder.h>


/** Message buffer size
 *
 */
#define MSG_BUF_SIZE  128


/** Message buffer head index
 *
 */
static ATOMIC_DECLARE (msg_head, 0);

/** Message buffer tail index
 *
 */
static ATOMIC_DECLARE (msg_tail, 0);

/** Message buffer
 *
 */
static volatile native_t msg_buffer [MSG_BUF_SIZE];


/** Wait queue for dorder events
 *
 */
static LIST_DECLARE (dorder_wait_queue);


/** Dorder signal
 *
 */
static ATOMIC_DECLARE (dorder_signal, 1);


/** Process dorder interrupt
 *
 * Consume messages from the dorder message buffer.
 *
 */
void dorder_handle (void)
{
	// TODO: Deassert dorder interrupt pending
	
	/*
	 * Read the messages from the buffer and
	 * process them.
	 */
	while (atomic_get (&msg_tail) != atomic_get (&msg_head)) {
		native_t msg_tail_index =
		    atomic_get (&msg_tail) % sizeof_array (msg_buffer);
		
		native_t msg = msg_buffer [msg_tail_index];
		atomic_add (&msg_tail, 1);
		
		dorder_receive (msg);
	}
}


/** Receive and process dorder message
 *
 * Receive and process a dorder message. If the message is a signal message,
 * then indicate the signal and wake up a thread that might be waiting for
 * the signal.
 *
 */
void dorder_receive (native_t msg)
{
	/*
	 * Print out the message (for debugging
	 * purposes)
	 */
	printk ("dorder: %x\n", msg);
	
	/*
	 * Process the messages
	 */
	if (msg == DORDER_MSG_SIGNAL) {
		// TODO: Set the signal atomic variable
		
		ipl_t status = query_and_disable_interrupts ();
		
		/*
		 * Wake up the first waiter.
		 */
		link_t *link = list_pop (&dorder_wait_queue);
		if (link != NULL) {
			struct thread *thread =
			    list_item (link, struct thread, wait_queue_link);
			
			// TODO: Remove the following line and
			//       wake up the thread here
			(void) thread;
		}
		
		conditionally_enable_interrupts (status);
	}
}


/** Send dorder interrupt
 *
 * Send dorder interrupt with a given message to the given
 * CPU.
 *
 * @param cpuid CPU identification number to send the interrupt
 *              to (range 0 - 31).
 * @param msg   Message to send.
 *
 */
void dorder_send (const uint32_t cpuid, native_t msg)
{
	/*
	 * This simplified dorder driver actually only supports
	 * a single CPU and a single message buffer.
	 */
	assert (cpuid == 0);
	
	/*
	 * Busy wait if the target CPU message buffer is full.
	 */
	while (atomic_get (&msg_tail) == atomic_get (&msg_head) + 1);
	
	/* Store the message to the buffer */
	native_t msg_head_index =
	    atomic_post_add (&msg_head, 1) % sizeof_array (msg_buffer);
	msg_buffer [msg_head_index] = msg;
	
	// TODO: Assert the dorder interrupt for the given CPU
}


/** Blocking wait for a dorder signal
 *
 * Wait until a dorder signal happens.
 *
 */
void dorder_wait (void)
{
	native_t signal;
	
	do {
		/* Non-zero signal means there is no pending signal */
		signal = atomic_test_and_set (&dorder_signal);
		
		if (signal != 0) {
			/*
			 * If there is no signal, wait for the signal passively.
			 */
			
			// TODO: Run this block with disabled interrupts
			
			// TODO: Put the current thread to sleep
		}
	} while (signal != 0);
}


/** Probe for a pending dorder signal
 *
 * Check the status of pending dorder signal.
 *
 * @return Zero if there is no pending dorder signal.
 * @return Non-zero if there is a pending dorder signal.
 *
 */
int dorder_probe (void)
{
	// TODO: Replace the following line with actually reading
	//       the signal flag
	native_t signal = 1;
	
	return !signal;
}
