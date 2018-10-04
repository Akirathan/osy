/***
 * Map test #2
 *
 * Change Log:
 * 03/12/02 ghort   created
 * 03/12/04 vilda   bug fixes, closer to ANSI C
 * 04/12/02 ghort   rewritten to reflect the new assignment spec
 * 04/12/08 pallas  sanitized and debugged and split to multiple files
 */

static const char * desc =
    "Map test #2\n\n"
    "Similar to map1 test, except multithreaded, this test performs\n"
    "various operations to exercise the management of virtual memory.\n"
    "Newly allocated areas of virtual memory are initialized with\n"
    "predefined values depending on the virtual address, which allows\n"
    "checking for errors resulting from mapping multiple virtual pages\n"
    "to the same physical page. The test performs these operations:\n\n"
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
    "    memory area and check wheter it contains expected value.\n"
    "  * access_bad - picks a random location in a randomly selected\n"
    "    hole in the virtual memory map and attempts to access it. The\n"
    "    should result in the calling thread getting killed.\n\n";


#include <api.h>
#include "../../include/defs.h"

#include "test.h"

#include "../../include/tst_area_list.h"
#include "../../include/tst_area_ops.h"
#include "../../include/tst_rand.h"

/****************************************************************************\
| PRIVATE DEFS                                                               |
\****************************************************************************/

/*
 * The number of thread groups to create. Threads within the same
 * group share the address space.
 */
#define THREAD_GROUPS_NUM  3

/*
 * The number of threads per thread group.
 */
#define THREADS_IN_GROUP_NUM  5


/*
 * Structure to hold data for test_thread function.
 */
struct test_thread_data {
	/*
	 * Group and thread ID.
	 */
	grpthr_id_t gtid;
	
	/*
	 * The memory map shared by the thread.
	 */
	tst_area_list_s * mem;
}; /* struct test_thread_data */


/****************************************************************************\
| PUBLIC DATA                                                                |
\****************************************************************************/

/*
 * Global console lock.
 */
struct mutex tst_console_lock;


/****************************************************************************\
| PRIVATE DATA                                                               |
\****************************************************************************/

/*
 * An array of IDs for group leader threads.
 */
static thread_t group_threads [THREAD_GROUPS_NUM];

/*
 * An array of IDs for individual threads in thread groups.
 */
static thread_t test_threads [THREAD_GROUPS_NUM] [THREADS_IN_GROUP_NUM];

/*
 * An array of parameters for each test_thread function.
 */
static struct test_thread_data test_thread_params [THREAD_GROUPS_NUM] [THREADS_IN_GROUP_NUM];


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

static struct action actions_64KB [] = {
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
do_phase (struct phase * phase, tst_area_list_s * mem, grpthr_id_t gtid)
{
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


/**
 * test_thread
 *
 * Thread function for testing the virtual memory management from
 * multiple threads in multiple thread groups.
 */
static void *
test_thread (struct test_thread_data * data)
{
	/*
	 * Run the individual phases of the test.
	 */
	unsigned int phaseno;
	for (phaseno = 0; phaseno < sizeof_array (phases); phaseno++) {
		struct phase * phase = &phases [phaseno];
		
		LOCK_CONSOLE;
			printk ("<%d,%d> Entering phase #%d. (%s)\n",
			    GET_GROUP_ID (data->gtid),
			    GET_THREAD_ID (data->gtid),
			    phaseno + 1, phase->name);
		UNLOCK_CONSOLE;
		
		do_phase (phase, data->mem, data->gtid);
		if (error_flag)
			break;
		
		LOCK_CONSOLE;
			printk ("<%d,%d> Phase #%d finished.\n",
			    GET_GROUP_ID (data->gtid),
			    GET_THREAD_ID (data->gtid),
			    phaseno + 1);
		UNLOCK_CONSOLE;
	} /* phases */
	
	return NULL;
} /* test_thread */


/**
* group_thread
*
* A thread function for a group leader, which creates a number of threads
* sharing the same address space to exercise the virtual memory management.
*/
static void *
group_thread (grpthr_id_t gid)
{
	int thrid;
	int grpid;
	
	LOCK_CONSOLE;
		printk ("<%d> Creating group of threads sharing the address "
		    "space.\n", GET_GROUP_ID (gid));
	UNLOCK_CONSOLE;
	
	// init
	tst_area_list_s mem;
	tst_area_list_init (& mem, ALLOWED_RANGE_START / FRAME_SIZE,
	    ALLOWED_RANGE_SIZE / FRAME_SIZE);
	
	/*
	 * Start the threads sharing the address space within the thread group.
	 */
	grpid = GET_GROUP_ID (gid);
	for (thrid = 0; thrid < THREADS_IN_GROUP_NUM; thrid++) {
		grpthr_id_t gtid = 0;
		SET_GROUP_ID (gtid, grpid);
		SET_THREAD_ID (gtid, thrid);
		
		/*
		 * Since we have to pass two arguments to the threads, we
		 * use use an auxiliary structure statically allocated in
		 * the nonmapped memory.
		 */
		struct test_thread_data * thread_data;
		thread_data = &test_thread_params [grpid] [thrid];
		thread_data->gtid = gtid;
		thread_data->mem = &mem;
		
		test_threads [grpid] [thrid] = robust_thread_create (
		    (thread_fn) test_thread, thread_data, 0);
	}
	
	/*
	 * Join the spawned threads.
	 *
	 * Ignore the return values because some of the threads might
	 * have got killed.
	 */
	for (thrid = 0; thrid < THREADS_IN_GROUP_NUM; thrid++) {
		thread_join (test_threads [grpid] [thrid], NULL);
	}
	
	return NULL;
} /* group_thread */


/****************************************************************************\
| PUBLIC CODE                                                                |
\****************************************************************************/

void
test_run (void)
{
	int grpid;
	
	printk (desc);
	
	// init
	INIT_CONSOLE_LOCK;
	
	/*
	 * Create thread groups with different address spaces.
	 */
	for (grpid = 0; grpid < THREAD_GROUPS_NUM; grpid++) {
		grpthr_id_t gid = 0;
		SET_GROUP_ID (gid, grpid);
		
		group_threads [grpid] = robust_thread_create (
		    (thread_fn) group_thread, (void *) gid, TF_NEW_VMM);
	}
	
	/*
	 * Join the spawned group leader threads. These should not die.
	 */
	for (grpid = 0; grpid < THREAD_GROUPS_NUM; grpid++) {
		robust_thread_join (group_threads [grpid]);
	}
	
	// print the result
	printk ("Test %s...\n", error_flag ? "failed" : "passed");
} /* test_run */
