/***
 * Area operations.
 *
 * A set of operations performed on the list of virtual memory areas.
 *
 * Change Log:
 * 2004/12/09 pallas  created
 */

#ifndef _TST_AREA_OPS_H_
#define _TST_AREA_OPS_H_

#include "tst_area_list.h"


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/*
 * This type carries identification of a thread group and a thread.
 * Thread id is in the lower 16 bits. Group id occupies the higher 16 bits.
 */
typedef unsigned int grpthr_id_t;

#define GRPTHR_ID_NULL           0
#define GET_GROUP_ID(gtid)       ((gtid) >> 16)
#define SET_GROUP_ID(gtid,val)   gtid = ((gtid) & 0xffff) | ((val) << 16)
#define GET_THREAD_ID(gtid)      ((gtid) & 0xffff)
#define SET_THREAD_ID(gtid,val)  gtid = ((gtid) & 0xffff0000) | ((val) & 0xffff)


/*
 * Structure holding the possible parameters of an action.
 */
struct action_param {
	int min_block_size;
	int max_block_size;
}; /* struct action_param */


/*
 * An action function.
 */
struct phase;

typedef void (* action_fn) (
	struct phase * phase,
	tst_area_list_s * mem,
	struct action_param * data,
	grpthr_id_t grpthrid,
	int cycles,
	int actionno
); /* action_fn */


/*
 * Structure describing an action.
 */
struct action {
	/*
	 * The action function to execute.
	 */
	action_fn fn;
	
	/*
	 * The probability of execution of the action.
	 */
	int prob;
	
	/*
	 * The parameters of the action function.
	 */
	struct action_param * data;
}; /* struct action */


/*
 * Structure describing a test phase.
 */
struct phase {
	/*
	 * Human readable description.
	 */
	char * name;
	
	/*
	 * Number of cycles to perform.
	 */
	int max_cycles;
	
	/*
	 * An array of actions to choose from in each cycle.
	 */
	struct action * actions;
	
	/*
	 * Number of actions in the array.
	 */
	int num_actions;
}; /* struct phase */


/*
 * Global flag which is set when an error is encountered
 * (overlapping blocks, incorrect data in a block, etc.).
 */
extern volatile int error_flag;


/****************************************************************************\
| PUBLIC FUNCTIONS                                                           |
\****************************************************************************/

/*
 * vma_map.
 */
extern void action_vma_map (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);
extern void action_vma_map_rand (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);
extern void action_vma_map_bad (struct phase *	phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);

/*
 * vma_unmap.
 */
extern void action_vma_unmap (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);
extern void action_vma_unmap_bad (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);

/*
 * Access.
 */
extern void action_access (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);
extern void action_access_bad (struct phase * phase, tst_area_list_s * mem,
    struct action_param * data, grpthr_id_t grpthrid, int cycles, int actionno);

#endif /* _TST_AREA_OPS_H_ */
