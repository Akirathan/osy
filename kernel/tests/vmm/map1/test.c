/***
 * Map test #1
 *
 * Change Log:
 * 03/11/20 ghort   created
 * 03/11/27 vilda   some bugs fixed
 * 03/12/02 ghort   polished
 * 04/12/02 ghort   rewritten to reflect the new assignment spec
 * 04/12/08 pallas  sanitized and debugged and split to multiple files
 */

static const char * desc =
    "Map test #1\n\n"
    "Similar to malloc1, this test performs various operations to\n"
    "exercise the management of virtual memory. Newly allocated areas\n"
    "of virtual memory are initialized with predefined values depending\n"
    "on the virtual address, which allows checking for errors resulting\n"
    "from mapping multiple virtual pages to the same physical page. The\n"
    "test performs the following operations:\n\n"
    "  * vma_map - allocates an area in predefined segment of virtual\n"
    "    address space using the VF_VA_AUTO flags. This means that the\n"
    "    operation always succeeds unless there is not enough memory.\n"
    "  * vma_map_rand - picks a random page in the virtual memory and\n"
    "    tries to allocate a virtual memory area starting at the page.\n"
    "  * vma_map_bad - attempts to allocate an areay that fully or at\n"
    "    least partiall overlaps with other, randomly choosen area.\n"
    "  * vma_unmap - picks a random virtual memory area and frees it.\n"
    "  * vma_unmap_bad - attempts to free a nonexistant area.\n"
    "  * access - picks a random location in a randomly selected virtual\n"
    "    memory area and check wheter it contains expected value.\n\n";


#include <api.h>

#include "test.h"

#include "../../include/tst_area_list.h"
#include "../../include/tst_area_ops.h"
#include "../../include/tst_rand.h"


/****************************************************************************\
| PRIVATE DATA                                                               |
\****************************************************************************/

/*
 * The test consist of several phases which differ in the size of blocks they
 * allocate and the probability of actions that take place in each phase.
 */

/*
 * Parameters for phase 1.
 */
static struct action_param data_vma_map_4KB = {
	.min_block_size = 1,
	.max_block_size = 1
};

static struct action actions_4KB [] = {
	{
		.fn = action_vma_map,
		.prob = 25,
		.data = &data_vma_map_4KB
	},
	{
		.fn = action_vma_map_rand,
		.prob = 30,
		.data = &data_vma_map_4KB
	},
	{
		.fn = action_vma_map_bad,
		.prob = 35,
		.data = &data_vma_map_4KB
	},
	{
		.fn = action_vma_unmap,
		.prob = 55,
		.data = NULL
	},
	{
		.fn = action_vma_unmap_bad,
		.prob = 60,
		.data = NULL
	},
	{
		.fn = action_access,
		.prob = 100,
		.data = NULL
	}
};


/*
 * Parameters for phase 2.
 */
static struct action_param data_vma_map_4KB_64KB = {
	.min_block_size = 1,
	.max_block_size = 16
};

static struct action actions_64KB[] = {
	{
		.fn = action_vma_map,
		.prob = 25,
		.data = &data_vma_map_4KB_64KB
	},
	{
		.fn = action_vma_map_rand,
		.prob = 30,
		.data = &data_vma_map_4KB_64KB
	},
	{
		.fn = action_vma_map_bad,
		.prob = 35,
		.data = &data_vma_map_4KB_64KB
	},
	{
		.fn = action_vma_unmap,
		.prob = 55,
		.data = NULL
	},
	{
		.fn = action_vma_unmap_bad,
		.prob = 60,
		.data = NULL
	},
	{
		.fn = action_access,
		.prob = 100,
		.data = NULL
	}
};


/*
 * Parameters for phase 3.
 */
static struct action_param data_vma_map_32KB_256KB = {
	.min_block_size = 8,
	.max_block_size = 64
};

static struct action actions_256KB [] = {
	{
		.fn = action_vma_map,
		.prob = 25,
		.data = &data_vma_map_32KB_256KB
	},
	{
		.fn = action_vma_map_rand,
		.prob = 30,
		.data = &data_vma_map_32KB_256KB
	},
	{
		.fn = action_vma_map_bad,
		.prob = 35,
		.data = &data_vma_map_32KB_256KB
	},
	{
		.fn = action_vma_unmap,
		.prob = 55,
		.data = NULL
	},
	{
		.fn = action_vma_unmap_bad,
		.prob = 60,
		.data = NULL
	},
	{
		.fn = action_access,
		.prob = 100,
		.data = NULL
	}
};


/*
 * Parameters of individual test phases.
 */
static struct phase phases [] = {
	{
		.name = "4KB memory blocks",
		.max_cycles = 20,
		.actions = actions_4KB,
		.num_actions = sizeof_array (actions_4KB)
	},
	{
		.name = "4KB..64KB memory blocks",
		.max_cycles = 20,
		.actions = actions_64KB,
		.num_actions = sizeof_array (actions_64KB)
	},
	{
		.name = "32KB..256KB memory blocks",
		.max_cycles = 20,
		.actions = actions_256KB,
		.num_actions = sizeof_array (actions_256KB)
	}
};


/****************************************************************************\
| PRIVATE CODE                                                               |
\****************************************************************************/

/**
 * do_phase
 *
 * Executes a single phase of the test.
 */
static void
do_phase (struct phase * phase, tst_area_list_s * mem)
{
	grpthr_id_t gtid = GRPTHR_ID_NULL;
	SET_GROUP_ID (gtid, 0);
	SET_THREAD_ID (gtid, 0);
	
	int cycles;
	for (cycles = 0; cycles < phase->max_cycles; cycles++) {
		int rnd;
		int actionno;
		
		/*
		 * Roll the dice and execute action the probability of which
		 * corresponds to the random number we obtained.
		 */
		rnd = tst_rand() % 100;
		for (actionno = 0; actionno < phase->num_actions; actionno++) {
			if (phase->actions [actionno].prob > rnd) {
				phase->actions [actionno].fn (phase, mem,
				    phase->actions [actionno].data,
				    gtid, cycles, actionno);
				break;
			}
		}
		
		if (error_flag)
			break;
	}
} /* do_phase */


/****************************************************************************\
| PUBLIC CODE                                                                |
\****************************************************************************/

void
test_run (void)
{
	printk (desc);
	
	// init
	tst_area_list_s mem;
	tst_area_list_init (&mem, ALLOWED_RANGE_START / FRAME_SIZE,
	    ALLOWED_RANGE_SIZE / FRAME_SIZE);
	
	/*
	 * Run the individual phases of the test.
	 */
	unsigned int phaseno;
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++) {
		struct phase * phase = &phases [phaseno];
		
		printk ("Entering phase #%d. (%s)\n", phaseno + 1,
		    phase->name);
		
		do_phase (phase, &mem);
		
		if (error_flag)
			break;
		
		printk ("Phase #%d finished.\n\n", phaseno + 1);
	} /* phases */
	
	
	// print the result
	printk ("Test %s...\n", error_flag ? "failed" : "passed");
} /* test_run */
