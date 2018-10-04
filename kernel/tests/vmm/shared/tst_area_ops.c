/***
 * Area operations.
 *
 * A set of operations performed on the list of virtual memory areas.
 *
 * Change Log:
 * 2004/12/09 pallas  created
 */

#include "../../include/tst_area_ops.h"
#include "../../include/tst_rand.h"

#include "test.h"

/****************************************************************************\
| PRIVATE DEFS                                                               |
\****************************************************************************/

#define PRINT_ACTION_COMMON_HEADER \
	printk ("<%d,%d> .. cycle %d (actionno: %d) -- ", \
	    GET_GROUP_ID (grpthrid), GET_THREAD_ID (grpthrid), \
	    cycles, actionno)


/****************************************************************************\
| PUBLIC DATA                                                                |
\****************************************************************************/

/*
 * Global flag which is set when an error is encountered
 * (overlapping blocks, incorrect data in a block, etc.).
 */
volatile int error_flag = 0;


/****************************************************************************\
| PRIVATE CODE (MISC)                                                        |
\****************************************************************************/

/**
 * result_name
 * @result: result to translate
 *
 * A human-friendly result name.
 */
static char * result_name (int result)
{
	char * name;
	
	switch (result) {
	case EOK:
		name = "EOK";
		break;
	case EINVAL:
		name = "EINVAL";
		break;
	case ENOMEM:
		name = "ENOMEM";
		break;
	default:
		name = "UNKNOWN";
	}
	
	return name;
} /* result_name */


/**
 * within_range
 *
 * Check if a page number and size in pages are still withing
 * an allowed page range.
 */
static inline int within_range (int page, int size)
{
	int page_start = ALLOWED_RANGE_START / FRAME_SIZE;
	int page_count = ALLOWED_RANGE_SIZE / FRAME_SIZE;
	
	if (page < page_start)
		return 0;
	
	if (page + size > page_start + page_count)
		return 0;
	
	return 1;
} /* within_range */


/****************************************************************************\
| PRIVATE CODE (MEMORY)                                                      |
\****************************************************************************/

/**
 * expected_value
 *
 * Computes the expected value of a byte located at position pos.
 */
static inline unsigned char
expected_value (struct phase * phase, tst_area_list_s * mem, void * pos)
{
	/*
	 * Phase is not used since data from different phases would differ,
	 * which is not what we actually want.
	 */
	return ((unsigned int) mem ^ (unsigned int) pos) & 0xff;
} /* expected_value */


/**
 * fill_block
 *
 * Fills a block of memory with precomputed values.
 */
static void
fill_block (struct phase * phase, tst_area_list_s *mem, void *start, size_t size)
{
	unsigned char * pos = (unsigned char *) start;
	
	while (size-- > 0) {
		(* pos) = expected_value (phase, mem, pos);
		pos++;
	}
} /* fill_block */


/**
 * verify_block
 *
 * Verifies that a block of memory contains expected values.
 */
static int
verify_block (struct phase * phase, tst_area_list_s *mem, void *start, size_t size)
{
	unsigned char * pos = (unsigned char *) start;
	
	while (size-- > 0) {
		if ((* pos) != expected_value (phase, mem, pos))
			return 0;
		
		pos++;
	}
	
	return 1;
} /* verify_block */


/****************************************************************************\
| PRIVATE CODE (RANDOM)                                                      |
\****************************************************************************/

/**
 * random_range
 *
 * Generates a random number in the range [start,end).
 */
static inline unsigned int
random_range (unsigned int start, unsigned int end)
{
	return start + tst_rand () % (end - start);
} /* random_range */


/**
 * random_page
 *
 * Returns a random page number in the allowed page range.
 */
static inline unsigned int
random_page (void)
{
	int page_start = ALLOWED_RANGE_START / FRAME_SIZE;
	int page_count = ALLOWED_RANGE_SIZE / FRAME_SIZE;
	
	return random_range (page_start, page_start + page_count);
} /* random_page */


/**
 * random_size
 *
 * Returns a random size in pages in range [min,max].
 */
static inline int
random_size (int min, int max)
{
	return random_range (min, max + 1);
} /* random_size */


/**
 * random_ptr
 *
 * Returns a random pointer to a memory location given by
 * starting page and size in pages.
 */
static inline unsigned char *
random_ptr (unsigned int page, unsigned int size)
{
	return (unsigned char *) (page * FRAME_SIZE + (tst_rand () % (size * FRAME_SIZE)));
} /* random_ptr */


/****************************************************************************\
| PUBLIC CODE (VMA_MAP)                                                      |
\****************************************************************************/

/**
 * action_vma_map
 *
 * Allocates an area in the designated segment using the VF_VA_AUTO flag,
 * which means that the action should always succeed unless there is no
 * memory available.
 */
void action_vma_map (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Allocate a random-sized virtual memory area.
	 */
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	size_t alloc_bytes = alloc_size * FRAME_SIZE;
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_map (size=%x)", alloc_bytes);
		
		void * alloc_ptr;
		int alloc_res = vma_map (&alloc_ptr, alloc_bytes, ALLOWED_RANGE_VFLAGS);
		printk (" = %s, ptr=%p\n", result_name (alloc_res), alloc_ptr);
		
		/*
		 * If the allocation failed, we just shrug our shoulders
		 * and return without making much fuss around it.
		 */
		if (alloc_res != EOK)
			goto exit_unlock_both;
		
		/*
		 * The allocation succeeded, check if the pointer is
		 * aligned to FRAME_SIZE.
		 */
		if ((unsigned int) alloc_ptr % FRAME_SIZE != 0) {
			printk ("<%d,%d>   ..  Error: Unaligned pointer "
			    "returned by vma_map.\n", GET_GROUP_ID (grpthrid),
			    GET_THREAD_ID (grpthrid));
			
			error_flag = 1;
			goto exit_unlock_both;
		}
		
		/*
		 * The pointer is OK, convert it to page number, add
		 * the area to the area list and check for overlaps.
		 */
		int alloc_page = (unsigned int) alloc_ptr / FRAME_SIZE;
		int expect_res = tst_area_list_add (mem, alloc_page, alloc_size);
		
		/*
		 * Check if there is an overlap of areas...
		 */
		if (expect_res == EINVAL) {
			/*
			 * There is an overlap of areas...
			 */
			printk ("<%d,%d>   ..  Error: Pointer to occupied "
			    "memory returned by vma_map.\n",
			    GET_GROUP_ID (grpthrid),
			    GET_THREAD_ID (grpthrid));
			
			error_flag = 1;
			goto exit_unlock_both;
		}
		
		/*
		 * If there was not enough memory to create a new list item,
		 * just free the allocated area, but don't report an error.
		 */
		if (expect_res == ENOMEM) {
			PRINT_ACTION_COMMON_HEADER;
			printk ("consistency vma_unmap (ptr=%p)", alloc_ptr);
			vma_unmap (alloc_ptr);
			goto exit_unlock_both;
		}
		
		/*
		 * If everything went smoothly, fill the allocated memory
		 * with a pattern that can be later checked.
		 */
		fill_block (phase, mem, alloc_ptr, alloc_bytes);
	
exit_unlock_both:
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
} /* action_vma_map */


/**
 * action_vma_map_rand
 *
 * Randomly picks a page in the virtual memory and tries to allocate memory
 * starting from it.
 */
void action_vma_map_rand (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Allocate a random-sized virtual memory area with random
	 * start address.
	 */
	unsigned int alloc_page = random_page ();
	void * alloc_ptr = (void *) (alloc_page * FRAME_SIZE);
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	size_t alloc_bytes = alloc_size * FRAME_SIZE;
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_map_rand (ptr=%p, size=%x)", alloc_ptr, alloc_bytes);
		
		int alloc_res = vma_map (&alloc_ptr, alloc_bytes, VF_USER_ADDR);
		printk (" = %s, ptr=%p\n", result_name (alloc_res), alloc_ptr);
		
		/*
		 * If there was not enough memory to allocate the area,
		 * we just return without reporting an error.
		 */
		if (alloc_res == ENOMEM)
			goto exit_unlock_both;
		
		/*
		 * In other cases, the result of vma_map should be the same
		 * as the result of tst_area_list_add, except when the result
		 * is ENOMEM, in which case we just make a consistency vma_unmap.
		 */
		int expect_res = tst_area_list_add (mem, alloc_page, alloc_size);
		if (expect_res == ENOMEM) {
			PRINT_ACTION_COMMON_HEADER;
			printk ("consistency vma_unmap (ptr=%p)", alloc_ptr);
			vma_unmap (alloc_ptr);
			goto exit_unlock_both;
		}
		
		if (alloc_res != expect_res) {
			/*
			 * There must have been an overlap.
			 */
			printk ("<%d,%d>   ..  Error: Pointer to occupied "
			    "memory returned by vma_map.\n",
			    GET_GROUP_ID (grpthrid),
			    GET_THREAD_ID (grpthrid));
			
			error_flag = 1;
			goto exit_unlock_both;
		}
		
		/*
		 * If everything went smoothly, check that vma_map returned
		 * the same pointer as requested and fill the area with
		 * predefined pattern.
		 */
		if (alloc_res == EOK) {
			if ((unsigned int) alloc_ptr != (alloc_page * FRAME_SIZE)) {
				printk ("<%d,%d>   ..  Error: Invalid pointer "
				    "returned by vma_map.\n",
				    GET_GROUP_ID (grpthrid),
				    GET_THREAD_ID (grpthrid));
				
				error_flag = 1;
				goto exit_unlock_both;
			}
			
			fill_block (phase, mem, alloc_ptr, alloc_bytes);
		}
	
exit_unlock_both:
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
} /* action_vma_map_rand */


/**
 * action_vma_map_bad
 *
 * Attempts to allocate an area that fully or partially overlaps with an
 * existing virtual memory area.
 */
void action_vma_map_bad (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_map_bad .. no memory to be checked\n");
		UNLOCK_CONSOLE;
		
		conditionally_enable_interrupts (ipl);
		return;
	}
	
	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;
	
	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, &area_page, &area_size);
	
	
	/*
	 * Select a random page/size overlapping the existing area.
	 */
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	int alloc_page = area_page + random_range (-alloc_size + 1, area_size);
	
	size_t alloc_bytes = alloc_size * FRAME_SIZE;
	void * alloc_ptr = (void *) (alloc_page * FRAME_SIZE);
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_map_bad (ptr=%p, size=%x)", alloc_ptr, alloc_bytes);
		
		/*
		 * If the randomly selected addresses are not within the
		 * allowed range, just return without reporting an error.
		 */
		if (!within_range (alloc_page, alloc_size)) {
			printk (", out of tested range\n");
			goto exit_unlock_both;
		}
		
		/*
		 * Attempt to allocate the memory and check the result.
		 */
		int alloc_res = vma_map (&alloc_ptr, alloc_bytes, VF_USER_ADDR);
		printk (" = %s\n", result_name (alloc_res));
		
		if (alloc_res == EOK) {
			printk ("   ..  Error: Allocated area that should not "
			    "be available.\n");
			error_flag = 1;
		}
	
exit_unlock_both:
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
} /* action_vma_map_bad */


/****************************************************************************\
| PUBLIC CODE (VMA_UNMAP)                                                    |
\****************************************************************************/

/**
 * action_vma_unmap
 *
 * Finds an allocated memory area and releases ii.
*/
void action_vma_unmap (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_unmap .. no memory to be released\n");
		UNLOCK_CONSOLE;
		
		conditionally_enable_interrupts (ipl);
		return;
	}
	
	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;
	
	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, &area_page, &area_size);
	
	/*
	 * Verify and release the area.
	 */
	void * release_ptr = (void *) (area_page * FRAME_SIZE);
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_unmap (ptr=%p)", release_ptr);
		
		/*
		 * Verify the area first.
		 */
		if (!verify_block (phase, mem, release_ptr, area_size * FRAME_SIZE)) {
			printk ("\n   ..  Error: The area to be freed has "
			    "been corrupted!\n");
			
			error_flag = 1;
			goto exit_unlock_both;
		}
		
		/*
		 * Release the area.
		 */
		int release_res = vma_unmap (release_ptr);
		printk (" = %s\n", result_name (release_res));
		
		if (release_res != EOK) {
			printk ("   ..  Error: Failed to free area expected "
			    "to be allocated!\n");
			
			error_flag = 1;
		}
	
exit_unlock_both:
	UNLOCK_CONSOLE;
	
	/*
	 * Delete the area from the list.
	 */
	tst_area_list_del (mem, area_idx);
	conditionally_enable_interrupts (ipl);
}


/**
 * action_vma_unmap_bad
 *
 * Attempts to release a virtual memory area with start address in
 * a hole in the virtual memory map.
 */
void action_vma_unmap_bad (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_unmap_bad .. no areas worth overlapping\n");
		UNLOCK_CONSOLE;
		
		conditionally_enable_interrupts (ipl);
		return;
	}
	
	
	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;
	
	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, &hole_page, &hole_size);
	
	/*
	 * Try to release the area.
	 */
	int release_page = hole_page + tst_rand () % hole_size;
	void * release_ptr = (void *) (release_page * FRAME_SIZE);
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_unmap_bad (ptr=%p)", release_ptr);
		
		int release_res = vma_unmap (release_ptr);
		printk (" = %s\n", result_name (release_res));
		
		if (release_res != EINVAL) {
			printk ("   ..  Error: Released memory area that was "
			    "supposed to be unallocated!\n");
			error_flag = 1;
		}
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
} /* action_vma_unmap_bad */


/****************************************************************************\
| PUBLIC CODE (ACCESS)                                                       |
\****************************************************************************/

/**
 * action_access
 *
 * Randomly picks an address in a randomly selected virtual memory area and
 * attempts to read a byte from it. The value should correspond to a value
 * expected to be read at that location.
 */
void action_access (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("access .. no areas to access\n");
		UNLOCK_CONSOLE;
		
		conditionally_enable_interrupts (ipl);
		return;
	}
	
	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;
	
	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, &area_page, &area_size);
	
	
	/*
	 * Pick a location within the area.
	 */
	unsigned char * access_ptr = random_ptr (area_page, area_size);
	unsigned char expected_val = expected_value (phase, mem, access_ptr);
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("access (ptr=%p) = ", access_ptr);
		printk ("%d, expected %d\n", *access_ptr, expected_val);
		
		if (*access_ptr != expected_val) {
			/*
			 * The value read does not match the expected value...
			 */
			printk ("<%d,%d>   ..  Error: Unexpected value "
			    "found at %p\n", GET_GROUP_ID (grpthrid),
			    GET_THREAD_ID (grpthrid), access_ptr);
			error_flag = 1;
		}
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
} /* action_access */


/**
 * action_access_bad
 *
 * Randomly picks an invalid location and attempts to read a byte from it.
 * Calling this action should result in the calling thread being killed.
 */
void action_access_bad (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid,
    int cycles, int actionno)
{
	ipl_t ipl = query_and_disable_interrupts ();
	
	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("access_bad .. no hole to access\n");
		UNLOCK_CONSOLE;
		
		conditionally_enable_interrupts (ipl);
		return;
	}
	
	
	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;
	
	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, &hole_page, &hole_size);
	
	/*
	 * Attempt to access a location within a hole in the memory map.
	 */
	unsigned char * access_ptr = random_ptr (hole_page, hole_size);
	
	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk("access_bad (ptr=%p) should cause exception\n", access_ptr);
	UNLOCK_CONSOLE;
	
	conditionally_enable_interrupts (ipl);
	(* access_ptr) = 0;
	
	/*
	 * This statement should not have been reached.
	 */
	
	LOCK_CONSOLE;
		printk ("<%d,%d>   ..  Error: Access to non-mapped memory did "
		    "not cause a fatal exception.\n",
		    GET_GROUP_ID (grpthrid), GET_THREAD_ID (grpthrid));
		
		error_flag = 1;
	UNLOCK_CONSOLE;
} /* action_access_bad */
