/***
 * Malloc test #1
 *
 * Change Log:
 * 2003/11/18 ghort   created
 * 2004/02/27 pallas  cleaned up and sanitized
 * 2004/03/16 pallas  fixed overlap checking
 * 2004/11/25 pallas  minor polishing
 * 2005/01/13 pallas  updated for user space API
 */

static const char * desc =
    "Tests the functionality of malloc and free functions.\n"
    "During the test we repeatedly allocate and release memory using\n"
    "malloc and free. The allocated chunks of memory must lie\n"
    "in the non-mapped segment of virtual address space and must not\n"
    "overlap. This is tested by writing chunk-specific data into an\n"
    "area after it has been allocated and checking the consistency of\n"
    "the data before releasing it.\n\n"
    "The test has several phases and subphases. During each phase, the\n"
    "test works with a specific chunk size in three randomized subphases.\n"
    "Each subphase randomly allocates and releases chunks of memory with\n"
    "different probability of allocations. The first phase mostly\n"
    "allocates memory, the middle phase allocates and releases memory\n"
    "with the same probability, and finally the third phases mostly\n"
    "releases memory.\n";


#include <librt.h>
#include "../../include/defs.h"

#include "../../include/tst_list.h"
#include "../../include/tst_rand.h"


/*
 * The memory between ALLOWED_RANGE_MIN and ALLOWED_RANGE_TOP - 1 is the
 * user space mapped through TLB. The allocated memory blocks should
 * come from this region only.
 */
#define ALLOWED_RANGE_MIN  0x00000000
#define ALLOWED_RANGE_TOP  0x80000000


/*
 * The test consists of several phases which differ in the size of blocks
 * they allocate. The size of blocks is given as a range of minimum and
 * maximum allowed size. Each of the phases is divided into 3 subphases which
 * differ in the probability of free and alloc actions. Second subphase is
 * started when malloc returns 'out of memory'. Third subphase is started
 * after a given number of cycles. The third subphase as well as the whole
 * phase ends when all memory blocks are released.
 */

/*
 * Subphase control structures: subphase termination conditions,
 * probabilities of individual actions, subphase control structure.
 */

typedef struct {
	unsigned int max_cycles;
	unsigned int no_memory;
	unsigned int no_allocated;
} sp_term_cond_s;

typedef struct {
	unsigned int alloc;
	unsigned int free;
} sp_action_prob_s;

typedef struct {
	char * name;
	sp_term_cond_s cond;
	sp_action_prob_s prob;
} subphase_s;


/*
 * Phase control structures: The minimum and maximum block size that
 * can be allocated during the phase execution, phase control structure.
 */

typedef struct {
	unsigned int min_block_size;
	unsigned int max_block_size;
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
		.name = "32B memory blocks",
		.alloc = {
			.min_block_size = 32,
			.max_block_size = 32,
			.top = 0
		},
		.subphases = subphases_32B
	},
	{
		.name = "128KB memory blocks",
		.alloc = {
			.min_block_size = 128 * 1024,
			.max_block_size = 128 * 1024,
			.top = 0
		},
		.subphases = subphases_128K
	},
	{
		.name = "2500B memory blocks",
		.alloc = {
			.min_block_size = 2500,
			.max_block_size = 2500,
			.top = 0
		},
		.subphases = subphases_default
	},
	{
		.name = "1B .. 250000B memory blocks",
		.alloc = {
			.min_block_size = 1,
			.max_block_size = 250000,
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
static unsigned int mem_allocated;
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
static void init_mem (void)
{
	mem_allocated = 0;
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

static int overlap_match (tst_list_head_t entry, overlap_match_data_t data)
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
static int test_overlap (void * addr, size_t size)
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
		printf ("Error: block <%p, %p> of size %u overlaps with\n"
		    "       block %p at <%p, %p> of size %u\n", addr,
		    (uint8_t *) addr + size, size, block, block->addr,
		    (uint8_t *) block->addr + block->size, block->size);
		return 1;
	}
	
	return 0;
} /* test_overlap */


/**
 * checked_malloc
 * @size: the amount of memory to allocate
 *
 * Allocates @size bytes of memory and check whether the chunk comes
 * from the user space mapped memory region and whether the chunk overlaps
 * with other, previously allocated, chunks.
 *
 * Returns NULL if the allocation failed. Sets the global error_flag to
 * TRUE if the allocation succeeded but is illegal.
 */
static void * checked_malloc (size_t size)
{
	void * data;
	
	// Allocate the chunk of memory
	data = malloc (size);
	if (data == NULL)
		return NULL;
	
	// Check the memory region
	// ALLOWED_RANGE_MIN is not checked, because size_t cannot
	// be negative
	if (((unsigned long) data > ALLOWED_RANGE_TOP) ||
	    ((unsigned long) data + size > ALLOWED_RANGE_TOP)) {
		printf ("Error: The allocated block <%p, %p> does not lie "
		    "in the user space mapped memory region.\n", data,
		    (uint8_t *) data + size);
		error_flag = TRUE;
	}
	
	// Check for overlaps with other chunks
	if (test_overlap (data, size)) {
		printf ("Error: The allocated block overlaps with another "
		    "previously allocated block.\n");
		error_flag = TRUE;
	}
	
	return data;
} /* checked_malloc */


/*
 * alloc_block
 * @size: the size of the memory block
 *
 * Allocates a block of memory of @size bytes and adds record about it into
 * the mem_blocks list. Returns a pointer to the block holder structure or
 * NULL if the allocation failed.
 *
 * If the allocation is illegal (e.g. the memory does not come from the
 * right region or some of the allocated blocks overlap with others), it
 * sets the global error_flag.
 */
static mem_block_t alloc_block (size_t size)
{
	mem_block_t block;
	
	// Allocate the block holder
	block = (mem_block_t) checked_malloc (sizeof (mem_block_s));
	if (block == NULL) {
		return NULL;
	}
	
	// Allocate the block memory
	block->addr = checked_malloc (size);
	if (block->addr == NULL) {
		free (block);
		return NULL;
	}
	
	block->size = size;
	
	// Register the allocated block
	tst_list_add_succ (&mem_blocks, &block->link);
	mem_allocated += size + sizeof (mem_block_s);
	mem_blocks_count++;
	
	return block;
} /* alloc_block */


/**
 * free_block
 * @block: block control structure
 *
 * Frees the block of memory and the block control structure allocated by
 * alloc_block. Sets the global error_flag if an error occurs.
 */
static void free_block (mem_block_t block)
{
	// Unregister the block
	tst_list_del (&block->link);
	mem_allocated -= block->size + sizeof (mem_block_s);
	mem_blocks_count--;
	
	// Free the memory
	free (block->addr);
	free (block);
} /* free_block */


/**
 * expected_value
 * @blk: memory block control structure
 * @pos: position in the memory block data area
 *
 * Computes the expected value of a byte located at @pos in a
 * memory block described by @blk.
 */
static inline uint8_t expected_value (mem_block_t blk, uint8_t * pos)
{
	return ((unsigned long) blk ^ (unsigned long) pos) & 0xff;
} /* expected_value */

/**
 * fill_block
 * @blk: memory block control structure
 *
 * Fills the memory block controlled by @blk with data.
 */
static void fill_block (mem_block_t blk)
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
static void check_block (mem_block_t blk)
{
	uint8_t * pos, * end;
	
	for (pos = blk->addr, end = pos + blk->size; pos < end; pos++) {
		if (*pos != expected_value (blk, pos)) {
			printf ("Error: Corrupted content of a data block.\n");
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
static mem_block_t get_random_block (void)
{
	if (mem_blocks_count == 0)
		return NULL;
	
	unsigned int blkidx = tst_rand () % mem_blocks_count;
	tst_list_head_t entry = tst_list_get_nth (&mem_blocks, blkidx);
	mem_block_t block = tst_list_entry (entry, mem_block_s, link);
	
	if (block == NULL) {
		printf ("Error: Corrupted list of allocated memory blocks.\n");
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

static void do_subphase (phase_s * phase, subphase_s * subphase)
{
	unsigned int cycles;
	for (cycles = 0; /* always */; cycles++) {
		
		if ((subphase->cond.max_cycles) &&
		    (cycles >= subphase->cond.max_cycles)) {
			/*
			 * We have performed the required number
			 * of cycles. End the current subphase.
			 */
			break;
		}
		
		/*
		 * Decide whether we alloc or free memory in this step.
		 */
		unsigned int rnd = tst_rand () % 100;
		if (rnd < subphase->prob.alloc) {
			/* We will allocate */
			unsigned int alloc;
			
			/* Compute a random number lying in interval <min_block_size,max_block_size> */
			alloc = phase->alloc.min_block_size +
			    (tst_rand () % (phase->alloc.max_block_size
			    - phase->alloc.min_block_size + 1));
			
			mem_block_t blk = alloc_block (alloc);
			RETURN_IF_ERROR;
			
			if (phase->alloc.top < mem_allocated)
				phase->alloc.top = mem_allocated;
			
			if (blk == 0) {
				printf ("..  cycle %u (%uB allocated) "
				    "-- failed to allocate %u bytes.\n",
				    cycles, mem_allocated, alloc);
				if (subphase->cond.no_memory) {
					/* We filled the memory. Proceed to next subphase */
					break;
				}
				
			} else {
				printf ("..  cycle %u (%uB allocated) "
				    "-- allocated %u bytes.\n",
				    cycles, mem_allocated, alloc);
				fill_block (blk);
			}
			
		} else if (rnd < subphase->prob.free) {
			/* We will free a memory block */
			
			mem_block_t blk = get_random_block();
			if (blk == 0) {
				printf("..  cycle %u (%uB allocated) "
				    "-- no block to be released.\n",
				    cycles, mem_allocated);
				if (subphase->cond.no_allocated) {
					/* We free all the memory. Proceed to next subphase. */
					break;
				}
				
			} else {
				printf("..  cycle %u (%uB allocated) "
				    "-- released %u bytes.\n",
				    cycles, mem_allocated, blk->size);
				check_block (blk);
				RETURN_IF_ERROR;
				
				free_block (blk);
				RETURN_IF_ERROR;
			}
		}
	} /* cycles */
	
	printf ("..  finished in %u cycles\n", cycles);
} /* do_subphase */

static void do_phase (phase_s * phase)
{
	unsigned int subno;
	for (subno = 0; subno < 3; subno++) {
		subphase_s * subphase = &phase->subphases [subno];
		
		printf (".. Sub-phase #%u (%s)\n", subno + 1, subphase->name);
		do_subphase (phase, subphase);
		RETURN_IF_ERROR;
	} /* subphases */
} /* do_phase */


int main (void)
{
	printf (desc);
	
	init_mem ();
	
	
	unsigned int phaseno;
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++) {
		phase_s * phase = &phases [phaseno];
		
		printf ("Entering phase #%u. (%s)\n", phaseno + 1,
		    phase->name);
		
		do_phase (phase);
		if (error_flag)
			break;
		
		printf ("Phase #%u finished.\n\n", phaseno + 1);
	} /* phases */
	
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++)
		printf ("Allocated in phase #%u: %u bytes\n", phaseno + 1,
		    phases[phaseno].alloc.top);
	
	if (error_flag) {
		printf("Test failed\n");
		return 1;
	}
	
	printf("\nTest passed...\n\n");
	return 0;
}
