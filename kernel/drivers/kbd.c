/**
 * @file kbd.c
 *
 * Keyboard.
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

#include <adt/plist.h>
#include <proc/thread.h>

#include <drivers/kbd.h>


/** Keyboard buffer size
 *
 */
#define KBD_BUF_SIZE  128


/** Keyboard buffer head index
 *
 */
static unsigned int kbd_head = 0;

/** Keyboard buffer tail index
 *
 */
static unsigned int kbd_tail = 0;

/** Keyboard buffer
 *
 */
static char kbd_buffer [KBD_BUF_SIZE];

/** Wait queue for sleeping on key read
 *
 */
static PLIST_DECLARE (kbd_wait_queue);


/** Process keyboard interrupt
 *
 * Read the keyboard register and store the key in the keyboard
 * buffer. If there is a thread waiting for a key then it is
 * woken up.
 *
 */
void kbd_handle (void)
{
	/*
	 * Read the character and put it in the buffer. If the
	 * buffer is full, throw the character away.
	 */
	
	// TODO: Replace the following line by actually reading
	//       the character from the device.
	char key = 0;
	unsigned int head_next = (kbd_head + 1) % sizeof_array (kbd_buffer);
	if (head_next == kbd_tail)
		return;
	
	kbd_buffer [kbd_head] = key;
	kbd_head = head_next;
	
	/*
	 * Wake up the first waiter.
	 */
	if (! plist_empty (&kbd_wait_queue)) {
		struct plist *item = plist_pop (&kbd_wait_queue);
		struct thread *thread =
		    plist_item (item, struct thread, wait_queue_link);
		
		// TODO: Replace the following line by actually waking
		//       up the thread.
		(void) thread;
	}
}


/** Read key (blocking)
 *
 * Read a key from the keyboard buffer. If the buffer is empty,
 * then the calling thread is blocked until a key is pressed.
 *
 * @return Read key.
 *
 */
char getc (void)
{
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU)
	
	/*
	 * If the buffer is empty, wait for a character to appear
	 * in the buffer.
	 */
	if (kbd_head == kbd_tail) {
		// TODO: Put the current thread to sleep until there
		//       is a character in the buffer.
	}
	
	/*
	 * Get a character from the buffer.
	 */
	int chr = kbd_buffer [kbd_tail];
	kbd_tail = (kbd_tail + 1) % sizeof_array (kbd_buffer);
	
	return chr;
}


/** Read key (non-blocking)
 *
 * Read a key from the keyboard buffer. If the buffer is empty,
 * then the return value is EAGAIN.
 *
 * @return Read key if the keyboard buffer is not empty.
 * @return EAGAIN if the keyboard buffer is empty.
 *
 */
int getc_try (void)
{
	// TODO: Make sure the rest of the function is
	//       executing atomically (on a single CPU)
	
	int chr = EAGAIN;
	if (kbd_head != kbd_tail)
		chr = getc ();
	
	return chr;
}
