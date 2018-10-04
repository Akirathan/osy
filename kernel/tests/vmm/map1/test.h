/***
 * Map test #1
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
 * Dummy console lock. This makes the phases usable both for single and
 * multithreaded tests.
 */
#define INIT_CONSOLE_LOCK
	#define LOCK_CONSOLE
#define UNLOCK_CONSOLE

#endif /* _TEST_H_ */
