/***
 * Map test #2
 *
 * Test configuration.
 *
 * Change Log:
 * 04/12/08 pallas  created
 */

#ifndef _TEST_H_
#define _TEST_H_

#include "../../include/defs.h"


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/*
 * Only the memory between within the interval
 * [ALLOWED_RANGE_START, ALLOWED_RANGE_START + ALLOWED_RANGE_SIZE) is
 * tested by the test.
 */
#define ALLOWED_RANGE_START  KSSEG_BASE
#define ALLOWED_RANGE_SIZE   KSSEG_SIZE

#define ALLOWED_RANGE_VFLAGS  VF_AUTO_KSSEG


/*
 * When printing to console we have to lock it so that the output from two
 * concurrent threads does not get mixed together. This, however, synchronizes
 * the access to memory maps which is undesirable.
 *
 * The test must pass when the console locking is turned off. For
 * debugging though, console locking is quite handy.
 */
#undef DO_CONSOLE_LOCK

#ifdef DO_CONSOLE_LOCK

extern struct mutex tst_console_lock;

#define INIT_CONSOLE_LOCK  mutex_init (& tst_console_lock)
#define LOCK_CONSOLE       mutex_lock (& tst_console_lock)
#define UNLOCK_CONSOLE     mutex_unlock (& tst_console_lock)

#else /* DO_CONSOLE_LOCK not defined */

#define INIT_CONSOLE_LOCK
#define LOCK_CONSOLE
#define UNLOCK_CONSOLE

#endif /* DO_CONSOLE_LOCK test */

#endif /* _TEST_H_ */
