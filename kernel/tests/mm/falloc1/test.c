/***
 * Frame allocator test #1
 *
 * Change Log:
 * 07/11/21 md created
 */

static const char * desc =
    "Frame allocator test #1\n\n"
    "Tests the functionality of kernel frame allocator.\n"
    "During the test we repeatedly allocate and release memory using\n"
    "frame_alloc() and frame_free(). The allocated frames must be accessible\n"
    "via the non-mapped segment of virtual address space and must not\n"
    "overlap. This is tested by writing frame-specific data into an\n"
    "area after it has been allocated and checking the consistency of\n"
    "the data before releasing it.\n\n"
    "The test has several phases and subphases. During each phase, the\n"
    "test works with a specific allocation size in three randomized subphases.\n"
    "Each subphase randomly allocates and releases frames of memory with\n"
    "different probability of allocations. The first phase mostly\n"
    "allocates memory, the middle phase allocates and releases memory\n"
    "with the same probability, and finally the third phases mostly\n"
    "releases memory.\n\n";


#include <api.h>
#include "../../include/defs.h"

#include "../../include/tst_list.h"
#include "../../include/tst_rand.h"


/*
 * The memory between ALLOWED_RANGE_MIN and ALLOWED_RANGE_TOP - 1 is
 * accessible via KSEG0.
 */
#define ALLOWED_RANGE_MIN  0x00000000
#define ALLOWED_RANGE_TOP  0x20000000


/*
 * Convert physical address to virtual address in KSEG0
 * and vice versa.
 */
#define PHYS2VIRT(addr)  (((uintptr_t) (addr)) + 0x80000000)
#define VIRT2PHYS(addr)  (((uintptr_t) (addr)) - 0x80000000)


/*
 * The test consists of several phases which differ in the count of frames
 * they allocate. The count of frames is given as a range of minimum and
 * maximum allowed count. Each of the phases is divided into 3 subphases which
 * differ in the probability of free and alloc actions. Second subphase is
 * started when frame_alloc returns 'out of memory'. Third subphase is started
 * after a given number of cycles. The third subphase as well as the whole
 * phase ends when all memory blocks are released.
 */

/*
 * Subphase control structures: subphase termination conditions,
 * probabilities of individual actions, subphase control structure.
 */

typedef struct {
	int max_cycles;
	int no_memory;
	int no_allocated;
} sp_term_cond_s;

typedef struct {
	int alloc;
	int free;
} sp_action_prob_s;

typedef struct {
	char * name;
	sp_term_cond_s cond;
	sp_action_prob_s prob;
} subphase_s;


/*
 * Phase control structures: The minimum and maximum count of frames that
 * can be allocated during the phase execution, phase control structure.
 */

typedef struct {
	unsigned int min_count;
	unsigned int max_count;
	unsigned int top;
} ph_alloc_size_s;

typedef struct {
	char * name;
	ph_alloc_size_s alloc;
	subphase_s * subphases;
} phase_s;


/*
 * Subphases are defined separately here. This is for two reasons:
 * 1) data are not duplicated, 2) we don't have to state beforehand
 * how many subphases a phase contains.
 */
static subphase_s subphases_32B [] = {
	{
		.name = "Allocation",
		.cond = {
			.max_cycles = 200,
			.no_memory = 1,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 90,
			.free = 100
		}
	},
	{
		.name = "Alloc/Dealloc",
		.cond = {
			.max_cycles = 200,
			.no_memory = 0,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 50,
			.free = 100
		}
	},
	{
		.name = "Deallocation",
		.cond = {
			.max_cycles = 0,
			.no_memory = 0,
			.no_allocated = 1,
		},
		.prob = {
			.alloc = 10,
			.free = 100
		}
	}
};

static subphase_s subphases_128K [] = {
	{
		.name = "Allocation",
		.cond = {
			.max_cycles = 0,
			.no_memory = 1,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 70,
			.free = 100
		}
	},
	{
		.name = "Alloc/Dealloc",
		.cond = {
			.max_cycles = 30,
			.no_memory = 0,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 50,
			.free = 100
		}
	},
	{
		.name = "Deallocation",
		.cond = {
			.max_cycles = 0,
			.no_memory = 0,
			.no_allocated = 1,
		},
		.prob = {
			.alloc = 30,
			.free = 100
		}
	}
};


static subphase_s subphases_default [] = {
	{
		.name = "Allocation",
		.cond = {
			.max_cycles = 0,
			.no_memory = 1,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 90,
			.free = 100
		}
	},
	{
		.name = "Alloc/Dealloc",
		.cond = {
			.max_cycles = 200,
			.no_memory = 0,
			.no_allocated = 0,
		},
		.prob = {
			.alloc = 50,
			.free = 100
		}
	},
	{
		.name = "Deallocation",
		.cond = {
			.max_cycles = 0,
			.no_memory = 0,
			.no_allocated = 1,
		},
		.prob = {
			.alloc = 10,
			.free = 100
		}
	}
};


/*
 * Phase definitions.
 */
static phase_s phases [] = {
	{
		.name = "1 frame",
		.alloc = {
			.min_count = 1,
			.max_count = 1,
			.top = 0
		},
		.subphases = subphases_32B
	},
	{
		.name = "32 frames",
		.alloc = {
			.min_count = 32,
			.max_count = 32,
			.top = 0
		},
		.subphases = subphases_128K
	},
	{
		.name = "2 frames",
		.alloc = {
			.min_count = 2,
			.max_count = 2,
			.top = 0
		},
		.subphases = subphases_default
	},
	{
		.name = "1 .. 100 frames",
		.alloc = {
			.min_count = 1,
			.max_count = 100,
			.top = 0
		},
		.subphases = subphases_default
	}
};


/*
 * Global error flag. The flag is set if an error
 * is encountered (overlapping blocks, inconsistent
 * block data, etc.)
 */
static int error_flag = 0;

/*
 * Memory accounting: the amount of allocated memory and the
 * number and list of allocated blocks.
 */
static unsigned int frames_allocated;
static unsigned int mem_blocks_count;

static tst_list_head_s mem_blocks;

typedef struct mem_block {
	void * addr;           // address of the start of the block
	size_t size;           // size of the memory block
	tst_list_head_s link;  // link to other blocks
} mem_block_s;

typedef mem_block_s * mem_block_t;


/**
 * init_mem
 *
 * Initializes the memory accounting structures.
 */
static void
init_mem (void)
{
	frames_allocated = 0;
	mem_blocks_count = 0;
	tst_list_init (&mem_blocks);
} /* init_mem */


/**
 * overlap_match
 * @entry:
 * @data:
 *
 * Match function for tst_list_find in test_overlap. Tests whether a block
 * specified in @data overlaps another block or its control structure
 * referenced by @entry.
 *
 * Returns 0 if the block does not overlap.
 */
typedef struct overlap_match_data {
	void * addr;
	size_t size;
} overlap_match_data_s;

typedef overlap_match_data_s * overlap_match_data_t;

static int
overlap_match (tst_list_head_t entry, overlap_match_data_t data)
{
	// memory block entry
	mem_block_t mblk = tst_list_entry (entry, mem_block_s, link);
	
	// entry block control structure <mbeg, mend)
	uint8_t * mbeg = (uint8_t *) mblk;
	uint8_t * mend = (uint8_t *) mblk + sizeof (mem_block_s);
	
	// entry block memory <bbeg, bend)
	uint8_t * bbeg = (uint8_t *) mblk->addr;
	uint8_t * bend = (uint8_t *) mblk->addr + mblk->size;
	
	// data block <dbeg, dend)
	uint8_t * dbeg = (uint8_t *) data->addr;
	uint8_t * dend = (uint8_t *) data->addr + data->size;
	
	// check for overlaps
	if ((mbeg >= dbeg && mbeg < dend) ||
	    (mend > dbeg && mend <= dend) ||
	    (bbeg >= dbeg && bbeg < dend) ||
	    (bend > dbeg && bend <= dend)) {
		return 1;
	}
	
	return 0;
} /* overlap_match */


/**
 * test_overlap
 * @addr: the initial address of the block
 * @size: the size of the block
 *
 * Tests whether a block starting at @addr overlaps with another, previously
 * allocated memory block or its control structure.
 *
 * Returns 0 if the block does not overlap.
 */
static int
test_overlap (void * addr, size_t size)
{
	mem_block_t block;
	tst_list_head_t entry;
	overlap_match_data_s match = {
		.addr = addr,
		.size = size
	};
	
	entry = tst_list_find (&mem_blocks,
	    (tst_list_match_fn) overlap_match, &match);
	
	if (entry) {
		block = tst_list_entry (entry, mem_block_s, link);
		printk ("Error: block <%p, %p> of size %u overlaps with\n"
		    "       block %p at <%p, %p> of size %u\n", addr,
		    (uint8_t *) addr + size, size, block, block->addr,
		    (uint8_t *) block->addr + block->size, block->size);
		return 1;
	}
	
	return 0;
} /* test_overlap */


/**
 * checked_frame_alloc
 * @count: the number of frames to allocate
 *
 * Allocates @count frames of physical memory and checks whether the
 * frames are accessible via the non-mapped memory region and whether
 * frames overlap with other, previously allocated, frames.
 *
 * Returns NULL if the allocation failed. Sets the global error_flag to
 * TRUE if the allocation succeeded but is illegal.
 */
static void *checked_frame_alloc (const size_t count)
{
	uintptr_t data = 0;
	const unsigned int flags = VF_AT_KSEG0 | VF_VA_AUTO;
	
	// Allocate the frames
	int alloc_res = frame_alloc (&data, count, flags);
	if (alloc_res != EOK)
		return NULL;
	
	// Check the memory region
	if (// (data < ALLOWED_RANGE_MIN) ||
	    (data > ALLOWED_RANGE_TOP) ||
	    (data + (count * FRAME_SIZE) > ALLOWED_RANGE_TOP)) {
		printk ("Error: The allocated block <%p, %p> does not lie "
		    "in the non-mapped memory region.\n", data,
		    (uint8_t *) data + (count * FRAME_SIZE));
		error_flag = TRUE;
	}
	
	// Check for overlapsi with other chunks
	if (test_overlap ((void *) PHYS2VIRT(data), count * FRAME_SIZE)) {
		printk ("Error: The allocated block overlaps with another "
		    "previously allocated block.\n");
		error_flag = TRUE;
	}
	
	return (void *) PHYS2VIRT(data);
} /* checked_frame_alloc */


/*
 * alloc_frames
 * @count: the number of frames to allocate
 *
 * Allocates a block of physical memory of @count frames and adds record about it into
 * the mem_blocks list. Returns a pointer to the block holder structure or
 * NULL if the allocation failed.
 *
 * If the allocation is illegal (e.g. the memory does not come from the
 * right region or some of the allocated frames overlap with others), it
 * sets the global error_flag.
 */
static mem_block_t
alloc_frames (unsigned int count)
{
	mem_block_t block;
	
	// Allocate the block holder
	block = (mem_block_t) malloc (sizeof (mem_block_s));
	if (block == NULL) {
		return NULL;
	}
	
	// Allocate the feames
	block->addr = checked_frame_alloc (count);
	if (block->addr == NULL) {
		free (block);
		return NULL;
	}
	
	block->size = count * FRAME_SIZE;
	
	// Register the allocated block
	tst_list_add_succ (&mem_blocks, &block->link);
	frames_allocated += count;
	mem_blocks_count++;
	
	return block;
} /* alloc_block */


/**
 * free_frames
 * @block: block control structure
 *
 * Frees frames of physical memory and the block control structure allocated by
 * alloc_block. Sets the global error_flag if an error occurs.
 */
static void
free_block (mem_block_t block)
{
	// Unregister the block
	tst_list_del (&block->link);
	frames_allocated -= block->size / FRAME_SIZE;
	mem_blocks_count--;
	
	// Free the memory
	frame_free (VIRT2PHYS(block->addr), block->size / FRAME_SIZE);
	free (block);
} /* free_block */


/**
 * expected_value
 * @blk: memory block control structure
 * @pos: position in the memory block data area
 *
 * Computes the expected value of a byte located at @pos in memory
 * block described by @blk.
 */
static inline uint8_t
expected_value (mem_block_t blk, uint8_t * pos)
{
	return ((unsigned long) blk ^ (unsigned long) pos) & 0xff;
} /* expected_value */

/**
 * fill_block
 * @blk: memory block control structure
 *
 * Fills the memory block controlled by @blk with data.
 */
static void
fill_block (mem_block_t blk)
{
	uint8_t * pos, * end;
	
	for (pos = blk->addr, end = pos + blk->size; pos < end; pos++) {
		*pos = expected_value (blk, pos);
	}
} /* fill_block */


/**
 * check_block
 * @blk: memory block control structure
 *
 * Checks whether the block @blk contains the data it was filled with.
 *
 * Sets global error_flag if an error occurs.
 */
static void
check_block (mem_block_t blk)
{
	uint8_t * pos, * end;
	
	for (pos = blk->addr, end = pos + blk->size; pos < end; pos++) {
		if (*pos != expected_value (blk, pos)) {
			printk ("Error: Corrupted content of a data block.\n");
			error_flag = 1;
			return;
		}
	}
} /* check_block */


/**
 * get_random_block
 *
 * Selects a random memory block from the list of allocated blocks.
 *
 * Returns the block control structure or NULL if the list is empty.
 */
static mem_block_t
get_random_block (void)
{
	if (mem_blocks_count == 0)
		return NULL;
	
	int blkidx = tst_rand () % mem_blocks_count;
	tst_list_head_t entry = tst_list_get_nth (&mem_blocks, blkidx);
	mem_block_t block = tst_list_entry (entry, mem_block_s, link);
	
	if (block == NULL) {
		printk ("Error: Corrupted list of allocated memory blocks.\n");
		error_flag = 1;
	}
	
	return block;
} /* get_random_block */


#define RETURN_IF_ERROR \
	{ \
		if (error_flag) { \
			return; \
		} \
	}

static void
do_subphase (phase_s * phase, subphase_s * subphase)
{
	int cycles;
	for (cycles = 0; /* always */; cycles++) {
		
		if (subphase->cond.max_cycles &&
			cycles >= subphase->cond.max_cycles) {
			/*
			 * We have performed the required number of
			 * cycles. End the current subphase.
			 */
			break;
		}
		
		/*
		 * Decide whether we alloc or free memory in this step.
		 */
		int rnd = tst_rand () % 100;
		if (rnd < subphase->prob.alloc) {
			/* We will allocate */
			int alloc;
			
			/* Compute a random number lying in interval <min_count,max_count> */
			alloc = phase->alloc.min_count +
			    (tst_rand () % (phase->alloc.max_count
			    - phase->alloc.min_count + 1));
			
			mem_block_t blk = alloc_frames (alloc);
			RETURN_IF_ERROR;
			
			if (phase->alloc.top < frames_allocated)
				phase->alloc.top = frames_allocated;
			
			if (blk == 0) {
				printk ("..  cycle %u (%u frames allocated) "
				    "-- failed to allocate %u frames.\n",
				    cycles, frames_allocated, alloc);
				if (subphase->cond.no_memory) {
					/* We filled the memory. Proceed to next subphase */
					break;
				}
			
			} else {
				printk ("..  cycle %u (%u frames allocated) "
				    "-- allocated %u frames.\n",
				    cycles, frames_allocated, alloc);
				fill_block (blk);
			}
			
		} else if (rnd < subphase->prob.free) {
			/* We will free a memory block */
			
			mem_block_t blk = get_random_block();
			if (blk == 0) {
				printk("..  cycle %u (%u frames allocated) "
				    "-- no block to be released.\n",
				    cycles, frames_allocated);
				if (subphase->cond.no_allocated) {
					/* We free all the memory. Proceed to next subphase. */
					break;
				}
			} else {
				printk("..  cycle %u (%u frames allocated) "
				    "-- released %u bytes.\n",
				    cycles, frames_allocated, blk->size);
				check_block (blk);
				RETURN_IF_ERROR;
				
				free_block (blk);
				RETURN_IF_ERROR;
			}
		}
	} /* cycles */
	
	printk ("..  finished in %u cycles\n", cycles);
} /* do_subphase */

static void
do_phase (phase_s * phase)
{
	unsigned int subno;
	for (subno = 0; subno < 3; subno++) {
		subphase_s * subphase = &phase->subphases [subno];
		
		printk (".. Sub-phase #%u (%s)\n", subno + 1, subphase->name);
		do_subphase (phase, subphase);
		RETURN_IF_ERROR;
	} /* subphases */
} /* do_phase */


void
test_run (void)
{
	printk (desc);
	
	init_mem ();
	
	unsigned int phaseno;
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++) {
		phase_s * phase = &phases [phaseno];
		
		printk ("Entering phase #%u. (%s)\n", phaseno + 1,
		    phase->name);
		
		do_phase (phase);
		if (error_flag)
			break;
		
		printk ("Phase #%u finished.\n\n", phaseno + 1);
	} /* phases */
	
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++)
		printk ("Allocated in phase #%u: %u frames\n", phaseno + 1,
		    phases[phaseno].alloc.top);
	
	printk ("Test %s...\n", error_flag ? "failed" : "passed");
} /* test_run */
