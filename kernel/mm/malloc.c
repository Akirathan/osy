/**
 * @file malloc.c
 *
 * Kernel memory allocator.
 *
 * The allocator expects the heap to reside in a continuous physical
 * memory ranges. Heap blocks have headers and footers and are kept
 * next to each other so that traversing can be done by adding or
 * subtracting the appropriate size.
 *
 * The allocation policy is first fit, blocks are merged on free.
 * The allocator uses the frame allocator interface to acquire
 * the continuous physical memory ranges which are used as the backing
 * store.
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

#include <adt/list.h>
#include <mm/falloc.h>
#include <lib/debug.h>

#include <mm/malloc.h>


/** Magic used in heap headers. */
#define HEAP_BLOCK_HEAD_MAGIC  0xBEEF0001

/** Magic used in heap footers. */
#define HEAP_BLOCK_FOOT_MAGIC  0xBEEF0002

/** Some maximum heap block size to avoid overflow. */
#define HEAP_BLOCK_SIZE_MAX    0x10000000


/** Generic memory alignment */
#define ALIGNMENT  4


/** Minimum heap size in frames */
#define HEAP_FRAMES  16


/** Heap structure
 *
 * This structure represents a single continuous
 * heap.
 *
 */
typedef struct {
	/** A heap is in the list of heaps */
	link_t link;
	
	/** Address of heap start */
	void *heap_start;
	
	/** Address of heap end */
	void *heap_end;
	
	/** Number of frames the heap occupies */
	size_t frames;
} heap_t;


/** Header of a heap block
 *
 */
typedef struct {
	/** Size of the block including header and footer */
	size_t size;
	
	/** Indication of a free block */
	bool free;
	
	/** Which heap this block belongs to */
	heap_t *heap;
	
	/**
	 * A magic value to detect overwrite of heap header.
	 * The value is at the end of the header because
	 * that is where writing past block start will
	 * do damage.
	 */
	uint32_t magic;
} heap_block_head_t;

/** Footer of a heap block
 *
 */
typedef struct {
	/**
	 * A magic value to detect overwrite of heap footer.
	 * The value is at the beginning of the footer
	 * because that is where writing past block
	 * end will do damage.
	 */
	uint32_t magic;
	
	/** Size of the block */
	size_t size;
} heap_block_foot_t;


/** List of heaps */
static list_t heap_list;


#ifdef NDEBUG

/** Dummy heap block check
 *
 * Do not actually do any checking if we are building
 * a non-debug kernel.
 *
 */
#define block_check(addr)

#else /* NDEBUG */

/** Check a heap block
 *
 * Verifies that the structures related to a heap block still contain
 * the magic constants. This helps detect heap corruption early on.
 *
 * @param addr Address of the block.
 *
 */
static void block_check (void *addr)
{
	/* Calculate the position of the header. */
	heap_block_head_t *head = (heap_block_head_t *) addr;
	
	/* Make sure the block belongs to its heap. */
	assert ((void *) head >= head->heap->heap_start);
	assert ((void *) head < head->heap->heap_end);
	
	/* Make sure the header is still intact. */
	assert (head->magic == HEAP_BLOCK_HEAD_MAGIC);
	
	/* Calculate the position of the footer. */
	heap_block_foot_t *foot =
	    (heap_block_foot_t *) (addr + head->size - sizeof (heap_block_foot_t));
	
	/* Make sure the footer is still intact. */
	assert (foot->magic == HEAP_BLOCK_FOOT_MAGIC);
	
	/* And one extra check for the fun of it. */
	assert (head->size == foot->size);
}

#endif /* NDEBUG */


/** Initialize a heap block
 *
 * Fills in the structures related to a heap block.
 *
 * @param addr Address of the block.
 * @param size Size of the block including the header and the footer.
 * @param free Indication of a free block.
 * @param heap Heap the block belongs to.
 *
 */
static void block_init (void *addr, size_t size, bool free, heap_t *heap)
{
	/* Calculate the position of the header and the footer. */
	heap_block_head_t *head = (heap_block_head_t *) addr;
	heap_block_foot_t *foot =
	    (heap_block_foot_t *) (addr + size - sizeof (heap_block_foot_t));
	
	/* Fill the header. */
	head->size = size;
	head->free = free;
	head->heap = heap;
	head->magic = HEAP_BLOCK_HEAD_MAGIC;
	
	/* Fill the footer. */
	foot->magic = HEAP_BLOCK_FOOT_MAGIC;
	foot->size = size;
}


/** Initialize the heap allocator
 *
 * Create the heap management structures.
 *
 */
void heap_init (void)
{
	list_init (&heap_list);
}


/** Allocate a new heap
 *
 * Allocate a new heap. The initial portion of the new
 * heap is immediatelly claimed as an occuptied block.
 *
 * @param size The of the block that should be immediatelly
 *             allocated from the new heap.
 *
 * @return The address of the allocated block or NULL when not
 *         enough memory.
 *
 */
static void *malloc_heap (const size_t size)
{
	/*
	 * Calculate the proper size of the new heap
	 * (with appropriate headers and alignment).
	 */
	size_t real_size = ALIGN_UP (size, ALIGNMENT) +
	    sizeof (heap_block_head_t) + sizeof (heap_block_foot_t);
	size_t split_limit = real_size +
	    sizeof (heap_block_head_t) + sizeof (heap_block_foot_t);
	size_t heap_size =
	    ALIGN_UP (real_size + sizeof (heap_t), FRAME_SIZE);
	
	/*
	 * Calcutate the number of frames required from
	 * the frame allocator. The new heap should be at
	 * least HEAP_FRAMES large.
	 */
	size_t frames = heap_size >> FRAME_WIDTH;
	if (frames < HEAP_FRAMES) {
		frames = HEAP_FRAMES;
		heap_size = frames << FRAME_WIDTH;
	}
	
	/*
	 * Ask the frame allocator for a continuous physical
	 * memory area for the new heap.
	 */
	uintptr_t phys;
	int rc = frame_alloc (&phys, frames, VF_VA_AUTO | VF_AT_KSEG0);
	if (rc != EOK)
		return NULL;
	
	/*
	 * Initialize the heap structure.
	 */
	heap_t *heap = (heap_t *) ADDR_IN_KSEG0 (phys);
	
	link_init (&heap->link);
	heap->heap_start = ((void *) heap) + sizeof (heap_t);
	heap->heap_end = ((void *) heap) + heap_size;
	heap->frames = frames;
	
	heap_block_head_t *cur = (heap_block_head_t *) heap->heap_start;
	size_t payload_heap_size = heap_size - sizeof (heap_t);
	
	/*
	 * Allocate the block from the newly created heap.
	 */
	if (payload_heap_size > split_limit) {
		void *next = ((void *) cur) + real_size;
		block_init (next, payload_heap_size - real_size, true, heap);
		block_init (cur, real_size, false, heap);
	} else
		block_init (cur, payload_heap_size, false, heap);
	
	list_append (&heap_list, &heap->link);
	
	return ((void *) heap->heap_start + sizeof (heap_block_head_t));
}


/** Allocate a memory block
 *
 * @param size The size of the block to allocate.
 *
 * @return The address of the block or NULL when not enough memory.
 *
 */
void *malloc (const size_t size)
{
	/*
	 * Checking for maximum size avoids errors due to
	 * overflow, which would be hard to debug.
	 */
	assert (size <= HEAP_BLOCK_SIZE_MAX);
	
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	/*
	 * We have to allocate a bit more to have room for
	 * header and footer. The size of the memory block
	 * also has to be 4 bytes aligned to avoid unaligned
	 * memory access exception while accessing the
	 * footer structure.
	 */
	size_t real_size = ALIGN_UP (size, ALIGNMENT) +
	    sizeof (heap_block_head_t) + sizeof (heap_block_foot_t);
	
	void *result = NULL;
	heap_block_head_t *smallest_diff_head = NULL;
	
	/* Iterate over all heaps */
	list_foreach (heap_list, heap_t, link, heap) {
		heap_block_head_t *pos = (heap_block_head_t *) heap->heap_start;
		size_t smallest_diff = 0;
		
		while ((void *) pos < heap->heap_end) {
			/* Make sure the heap is not corrupted. */
			block_check (pos);
			
			/* Try to find a block that is free and large enough. */
			if ((pos->free) && (pos->size >= real_size)) {

				size_t curr_diff = pos->size - real_size;
				/* Check if the block fits best. */
				if (curr_diff < smallest_diff) {
					smallest_diff = curr_diff;
					smallest_diff_head = pos;
				}
			}
			
			/* Advance to the next block. */
			pos = (heap_block_head_t *) (((void *) pos) + pos->size);
		}
	}

	/* We managed to find some "best fit" block. */
	if (smallest_diff_head != NULL) {
		result = ((void *) smallest_diff_head) + sizeof(heap_block_head_t);
	}
	
	/*
	 * There is not enough empty space in the existing
	 * heaps. Try to acquire a new heap and allocate the
	 * block from it.
	 */
	if (result == NULL)
		result = malloc_heap (size);
	
	conditionally_enable_interrupts (state);
	
	return result;
}


/** Allocate a memory block
 *
 * Unlike standard malloc, this routine never returns NULL to
 * indicate an out of memory condition. Recovering from an out
 * of memory condition in the kernel is difficult and we never
 * really need it in the example kernel.
 *
 * Warning: Memory management implementation is not SMP safe.
 *
 * @param size The size of the block to allocate.
 * @return The address of the block.
 *
 */
void *safe_malloc (const size_t size)
{
	void *result = malloc (size);
	assert (result != NULL);
	return (result);
}


/** Free a memory block
 *
 * @param addr The address of the block.
 */
void free (const void *addr)
{
	/* Disable interrupts while accessing shared structures. */
	ipl_t state = query_and_disable_interrupts ();
	
	/* Calculate the position of the header. */
	heap_block_head_t *head =
	    (heap_block_head_t *) (addr - sizeof (heap_block_head_t));
	
	/* Make sure the block is not corrupted and not free. */
	block_check (head);
	assert (!head->free);
	
	/* Get the heap the block belongs to. */
	heap_t *heap = head->heap;
	
	/* Mark the block itself as free. */
	head->free = true;
	
	/* Look at the next block. If it is free, merge the two. */
	heap_block_head_t *next_head =
	    (heap_block_head_t *) (((void *) head) + head->size);
	
	if ((void *) next_head < heap->heap_end) {
		block_check (next_head);
		if (next_head->free)
			block_init (head, head->size + next_head->size, true, heap);
	}
	
	/* Look at the previous block. If it is free, merge the two. */
	if ((void *) head > heap->heap_start) {
		heap_block_foot_t *prev_foot =
		    (heap_block_foot_t *) (((void *) head) - sizeof (heap_block_foot_t));
		
		heap_block_head_t *prev_head =
		    (heap_block_head_t *) (((void *) head) - prev_foot->size);
		
		block_check (prev_head);
		
		if (prev_head->free)
			block_init (prev_head, prev_head->size + head->size, true, heap);
	}
	
	/*
	 * Check whether the entire heap is just one free block.
	 * If this is the case then release it entirely.
	 */
	head = (heap_block_head_t *) heap->heap_start;
	if ((head->free) && (heap->heap_start + head->size == heap->heap_end)) {
		list_remove (&heap->link);
		
		/*
		 * Free the physical frames backing the heap.
		 */
		uintptr_t phys = ADDR_FROM_KSEG0 ((uintptr_t) heap);
		int rc = frame_free (phys, heap->frames);
		if (rc != EOK)
			panic ("Unable to release heap.");
	}
	
	conditionally_enable_interrupts (state);
}
