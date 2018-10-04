/***
 * Disk test #1
 *
 * Change Log:
 * 15/12/01 md created
 */

static const char * desc =
    "Disk test #1\n\n"
    "Tests the functionality of kernel disk device driver.\n\n";


#include <api.h>
#include "../../include/defs.h"
#include "../../include/tst_rand.h"


#define SEED_DEFAULT  0x00
#define SEED_FORCE    0x55

#define PHASES  10

/** Number of blocks
 *
 */
static size_t blocks;


/** Pattern seeds of individual blocks
 *
 */
static uint8_t *seeds;


/** Global error flag
 *
 * If set to false, some tests have failed.
 *
 */
static ATOMIC_DECLARE (test_flag, true);


/** Busy block
 *
 * The block that is currently being updated
 * and should be therefore avoided in the reading
 * thread.
 *
 */
static ATOMIC_DECLARE (busy, -1);


/** Compute expected value
 *
 * Compute the expected pattern value.
 *
 * @param block  Block number
 * @param offset Offset within the block
 * @param seed   Pattern seed
 *
 * @return Pattern value
 *
 */
static inline uint8_t expected_value (size_t block, size_t offset, uint8_t seed)
{
	return ((unsigned long) seed ^ (unsigned long) block ^
	    (unsigned long) offset) & 0xff;
}


/** Fill block with pattern
 *
 * Fill the block with test pattern.
 *
 * @param block Block number
 * @param seed  Pattern seed
 * @param data  Block memory buffer
 *
 */
static void fill_block (size_t block, uint8_t seed, uint8_t *data)
{
	for (size_t offset = 0; offset < DISK_BLOCK_SIZE; offset++) {
		data[offset] = expected_value (block, offset, seed);
	}
}


/** Check block pattern
 *
 * Check the expected test pattern in the block.
 *
 * @param block Block number
 * @param seed  Pattern seed
 * @param data  Block memory buffer
 *
 * @return True if the pattern is correct
 * @return False if the pattern is not correct
 *
 */
static int check_block (size_t block, uint8_t seed, uint8_t *data)
{
	for (size_t offset = 0; offset < DISK_BLOCK_SIZE; offset++) {
		if (data[offset] != expected_value (block, offset, seed)) {
			printk ("Error: Corrupted content of block %u.\n", block);
			return false;
		}
	}
	
	return true;
}


/** Sequential reading test
 *
 * Read all device blocks sequentially and check whether
 * they contain the expected test pattern.
 *
 * @return True if the pattern is correct
 * @return False if the pattern is not correct
 *
 */
static int test_sequential_read (void)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t block = 0; block < blocks; block++) {
		int rc = disk_read (block, data);
		if (rc != EOK) {
			printk ("Error reading block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seeds [block], data))
			return false;
	}
	
	return true;
}


/** Random reading test
 *
 * Read some device blocks randomly and check whether
 * they contain the expected test pattern.
 *
 * @return True if the pattern is correct
 * @return False if the pattern is not correct
 *
 */
static int test_random_read (void)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t cnt = 0; cnt < blocks; cnt++) {
		size_t block = tst_rand () % blocks;
		
		int rc = disk_read (block, data);
		if (rc != EOK) {
			printk ("Error reading block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seeds [block], data))
			return false;
	}
	
	return true;
}


/** Sequential write test
 *
 * Write all device blocks sequentially and store a test
 * pattern to them.
 *
 * @param seed  Pattern seed
 *
 * @return True if the writing was successful
 * @return False if the writing was not successful
 *
 */
static int test_sequential_write (uint8_t seed)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t block = 0; block < blocks; block++) {
		fill_block (block, seed, data);
		
		int rc = disk_write (block, data);
		if (rc != EOK) {
			printk ("Error writing block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seed, data))
			return false;
		
		seeds [block] = seed;
	}
	
	return true;
}


/** Random write test
 *
 * Write some device blocks random and store a test
 * pattern to them.
 *
 * @param seed  Pattern seed
 *
 * @return True if the writing was successful
 * @return False if the writing was not successful
 *
 */
static int test_random_write (uint8_t seed)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t cnt = 0; cnt < blocks; cnt++) {
		size_t block = tst_rand () % blocks;
		
		fill_block (block, seed, data);
		
		int rc = disk_write (block, data);
		if (rc != EOK) {
			printk ("Error writing block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seed, data))
			return false;
		
		seeds [block] = seed;
	}
	
	return true;
}


/** Sequential write test with random seeds
 *
 * Write all device blocks sequentially and store a test
 * pattern to them (with a random seed).
 *
 * @return True if the writing was successful
 * @return False if the writing was not successful
 *
 */
static int test_sequential_write_random (void)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t block = 0; block < blocks; block++) {
		uint8_t seed = tst_rand () & 0xff;
		
		fill_block (block, seed, data);
		
		int rc = disk_write (block, data);
		if (rc != EOK) {
			printk ("Error writing block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seed, data))
			return false;
		
		seeds [block] = seed;
	}
	
	return true;
}


/** Random write test with random seeds
 *
 * Write some device blocks random and store a test
 * pattern to them (with a random seed).
 *
 * @return True if the writing was successful
 * @return False if the writing was not successful
 *
 */
static int test_random_write_random (void)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (size_t cnt = 0; cnt < blocks; cnt++) {
		size_t block = tst_rand () % blocks;
		uint8_t seed = tst_rand () & 0xff;
		
		fill_block (block, seed, data);
		
		int rc = disk_write (block, data);
		if (rc != EOK) {
			printk ("Error writing block %u.\n", block);
			return false;
		}
		
		if (!check_block (block, seed, data))
			return false;
		
		seeds [block] = seed;
	}
	
	return true;
}

/** Restore the disk content to default
 *
 * Restore the disk content using the default pattern.
 *
 * @return True if all operations were successful
 * @return False if not all operations were successful
 *
 */
static int cleanup (void)
{
	printk ("Sequential write (reseting to default seed) ... ");
	
	if (!test_sequential_write (SEED_DEFAULT))
		return false;
	
	printk ("OK\n");
	
	printk ("Sequential read (final check) ... ");
	
	if (!test_sequential_read ())
		return false;
	
	printk ("OK\n");
	
	return true;
}


/** Sequential read/write test
 *
 * @return True if all operations were successful
 * @return False if not all operations were successful
 *
 */
static int test_sequential (void)
{
	printk ("Sequential read (default seed) ... ");
	
	if (!test_sequential_read ())
		return false;
	
	printk ("OK\n");
	
	printk ("Sequential write (forced seed) ... ");
	
	if (!test_sequential_write (SEED_FORCE))
		return false;
	
	printk ("OK\n");
	
	printk ("Sequential read (forced seed) ... ");
	
	if (!test_sequential_read ())
		return false;
	
	printk ("OK\n");
	
	printk ("Sequential write (random seed) ... ");
	
	if (!test_sequential_write_random ())
		return false;
	
	printk ("OK\n");
	
	printk ("Sequential read (random seed) ... ");
	
	if (!test_sequential_read ())
		return false;
	
	printk ("OK\n");
	
	return cleanup ();
}


/** Random read/write test
 *
 * @return True if all operations were successful
 * @return False if not all operations were successful
 *
 */
static int test_random (void)
{
	printk ("Random read (default seed) ... ");
	
	if (!test_random_read ())
		return false;
	
	printk ("OK\n");
	
	printk ("Random write (forced seed) ... ");
	
	if (!test_random_write (SEED_FORCE))
		return false;
	
	printk ("OK\n");
	
	printk ("Random read (forced seed) ... ");
	
	if (!test_random_read ())
		return false;
	
	printk ("OK\n");
	
	printk ("Random write (random seed) ... ");
	
	if (!test_random_write_random ())
		return false;
	
	printk ("OK\n");
	
	printk ("Random read (random seed) ... ");
	
	if (!test_random_read ())
		return false;
	
	printk ("OK\n");
	
	return cleanup ();
}


/** Random read thread
 *
 */
static void *thread_read (void *arg)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (unsigned int phase = 0; phase < PHASES; phase++) {
		for (size_t cnt = 0; cnt < blocks; cnt++) {
			/*
			 * The random number generator and seed storage
			 * access is not thread-safe.
			 */
			ipl_t status = query_and_disable_interrupts ();
			size_t block = tst_rand () % blocks;
			uint8_t seed = seeds [block];
			conditionally_enable_interrupts (status);
			
			/*
			 * Skip reading a block that is currently in the
			 * process of manipulation.
			 */
			if (atomic_get (&busy) == (native_t) block)
				continue;
			
			int rc = disk_read (block, data);
			if (rc != EOK) {
				printk ("Error reading block %u.\n", block);
				atomic_set (&test_flag, false);
				return NULL;
			}
			
			if (!check_block (block, seed, data)) {
				atomic_set (&test_flag, false);
				return NULL;
			}
		}
		
		printk ("R");
	}
	
	return NULL;
}


/** Random write thread
 *
 */
static void *thread_write (void *arg)
{
	uint8_t data[DISK_BLOCK_SIZE];
	
	for (unsigned int phase = 0; phase < PHASES; phase++) {
		for (size_t cnt = 0; cnt < blocks; cnt++) {
			/*
			 * The random number generator is not thread-safe.
			 */
			ipl_t status = query_and_disable_interrupts ();
			size_t block = tst_rand () % blocks;
			uint8_t seed = tst_rand () & 0xff;
			conditionally_enable_interrupts (status);
			
			fill_block (block, seed, data);
			
			/*
			 * Indicate that the block is busy.
			 */
			atomic_set (&busy, block);
			
			int rc = disk_write (block, data);
			if (rc != EOK) {
				printk ("Error writing block %u.\n", block);
				atomic_set (&test_flag, false);
				return NULL;
			}
			
			if (!check_block (block, seed, data)) {
				atomic_set (&test_flag, false);
				return NULL;
			}
			
			/*
			 * The seed storage access is not thread-safe.
			 */
			status = query_and_disable_interrupts ();
			seeds [block] = seed;
			conditionally_enable_interrupts (status);
			
			/*
			 * Only after we update both the disk block
			 * and the seed storage the block is no longer
			 * busy.
			 */
			atomic_set (&busy, -1);
		}
		
		printk ("W");
	}
	
	return NULL;
}


void test_run (void)
{
	printk (desc);
	
	int rc = disk_get_nblocks (&blocks);
	if (rc != EOK) {
		printk ("Unable to determine number of blocks.\n");
		return;
	}
	
	seeds = (uint8_t *) safe_malloc (blocks);
	for (size_t block = 0; block < blocks; block++)
		seeds[block] = SEED_DEFAULT;
	
	printk ("Trying to read beyond the device end ... ");
	
	rc = disk_read (blocks, NULL);
	if (rc != EINVAL) {
		printk ("failed\n");
		return;
	}
	
	printk ("OK\n");
	
	printk ("Trying to write beyond the device end ... ");
	
	rc = disk_write (blocks, NULL);
	if (rc != EINVAL) {
		printk ("failed\n");
		return;
	}
	
	printk ("OK\n");
	
	if (!test_sequential ())
		return;
	
	if (!test_random ())
		return;
	
	printk ("Parallel access ... ");
	
	thread_t read_thread = robust_thread_create (thread_read, NULL, 0);
	thread_t write_thread = robust_thread_create (thread_write, NULL, 0);
	
	if (thread_join (read_thread, NULL) != EOK) {
		printk ("Error joining the reading thread.\n");
		return;
	}
	
	if (thread_join (write_thread, NULL) != EOK) {
		printk ("Error joining the writing thread.\n");
		return;
	}
	
	if (atomic_get (&test_flag) != true)
		return;
	
	printk ("\n");
	
	if (!cleanup ())
		return;
	
	printk ("Test passed...\n");
}
