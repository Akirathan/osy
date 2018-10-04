/**
 * Doubly-linked list.
 *
 * This is a simplified version of the Linux kernel
 * list implementation with minor tweaks.
 *
 * Change Log:
 * 03/10/23 pallas  created
 * 03/11/18 ghort   added function tst_list_get_nth, added some macros from
 *                  klib.h to satisfy dependencies on klib.h
 */

#ifndef _TST_LIST_H_
#define _TST_LIST_H_


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/*
 * List head/entry structure.
 */
typedef struct tst_list_head {
	struct tst_list_head * prev;
	struct tst_list_head * next;
} tst_list_head_s;

typedef tst_list_head_s * tst_list_head_t;


/*
 * Static list head/entry initializer.
 */
#define TST_LIST_HEAD_INIT(name) \
	{ \
		.prev = &(name), \
		.next = &(name) \
	}

/**
 * NULL
 *
 * Defines a null pointer.
 */
#ifndef NULL
	#define NULL  ((void *) 0)
#endif /* NULL */

/*
 * Various types of visitor/observer functions.
 */
typedef void (* tst_list_destroy_fn) (tst_list_head_t entry);
typedef void (* tst_list_visit_fn) (tst_list_head_t entry, void * data);
typedef int (* tst_list_match_fn) (tst_list_head_t entry, void * data);


/****************************************************************************\
| PRIVATE CODE                                                               |
\****************************************************************************/

/**
 * offset_of
 *
 * @type:   the type of the container struct a member is embedded in.
 * @member: the name of the member within the struct.
 *
 * Computes an offset of a member in a struct.
 */
#define offset_of(type, member) \
	((size_t) &((type *) 0)->member)

/**
 * container_of
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct a member is embedded in.
 * @member: the name of the member within the struct.
 *
 * Casts a member of a structure out to the containing structure.
 */
#define container_of(ptr, type, member) \
	({ \
		const typeof (((type *) 0)->member) * __mptr = (ptr); \
		(type *) ((char *) __mptr - offset_of (type, member)); \
	})


/**
 * __tst_list_add
 *
 * Internal function which inserts a new entry between two consecutive
 * entries. There can be a moment when the new item is visible during
 * backward but not during forward traversal, but this should be
 * handled by the caller.
 */
static inline void
__tst_list_add (tst_list_head_t new, tst_list_head_t prev, tst_list_head_t next)
{
	new->next = next;
	new->prev = prev;
	
	next->prev = new;
	prev->next = new;
} /* __tst_list_add */


/**
 * __tst_list_del
 *
 * Internal function which links together two entries, removing any
 * entries existing between the two from the list.
 *
 * Again, we link the backward pointer fist because lists are less
 * often traversed backwards. When we link the forward entry, the
 * list is completely linked. The caller should make sure this
 * is not a problem.
 */
static inline void
__tst_list_del (tst_list_head_t prev, tst_list_head_t next)
{
	next->prev = prev;
	prev->next = next;
} /* __tst_list_del */


/****************************************************************************\
| PUBLIC CODE                                                                |
\****************************************************************************/

/**
 * tst_list_init
 * @head: the list head to initialize
 *
 * Initialize the list. An empty list head
 * points back to itself for easy runtime checks.
 */
static inline void
tst_list_init (tst_list_head_t head)
{
	head->prev = (tst_list_head_t) head;
	head->next = (tst_list_head_t) head;
} /* tst_list_init */


/**
 * tst_list_empty
 *
 * Tests whether a list is empty.
 */
static inline int
tst_list_empty (tst_list_head_t head)
{
	return head->next == head;
} /* tst_list_empty */


/**
 * tst_list_entry
 * @ptr:        the &tst_list_head_s pointer
 * @type:       the type of the struct tst_list_head_s is embedded in
 * @member:     the name of the tst_list_head_s field within the struct
 *
 * Get a typed structure from the given list entry.
 */
#define tst_list_entry(ptr, type, member) \
	container_of (ptr, type, member)


/**
 * tst_list_add_succ
 * @head: the list head to add the entry after
 * @new:  the new list entry
 *
 * Adds a successor entry to the list after the specified head.
 */
static inline tst_list_head_t
tst_list_add_succ (tst_list_head_t head, tst_list_head_t new)
{
	__tst_list_add (new, head, head->next);
	return new;
} /* tst_list_add_succ */


/**
 * tst_list_add_pred
 * @head: the list head to add the entry before
 * @new:  the new list entry
 *
 * Adds a predecessor entry to the list before the specified head.
 */
static inline tst_list_head_t
tst_list_add_pred (tst_list_head_t head, tst_list_head_t new)
{
	__tst_list_add (new, head->prev, head);
	return new;
} /* tst_list_add_pred */


/**
 * tst_list_del
 *
 * Deletes a specific entry from the list and reinitializes
 * the entry so that tst_list_empty () returns true on it.
 */
static inline tst_list_head_t
tst_list_del (tst_list_head_t entry)
{
	__tst_list_del (entry->prev, entry->next);
	
	entry->next = entry;
	entry->prev = entry;
	return entry;
} /* tst_list_del */


/**
 * tst_list_del_succ
 * @head: the list head to delete from
 *
 * Removes the successor of the specified list head from the list
 * and returns it to the caller.
 */
static inline tst_list_head_t
tst_list_del_succ (tst_list_head_t head)
{
	tst_list_head_t entry = head->next;
	tst_list_del (entry);
	return entry;
} /* tst_list_del_succ */


/**
 * tst_list_del_pred
 * @head: the list head to delete from
 *
 * Removes the predecessor of the specified list head from the
 * list and returns it to the caller.
 */
static inline tst_list_head_t
tst_list_del_pred (tst_list_head_t head)
{
	tst_list_head_t entry = head->prev;
	tst_list_del (entry);
	return entry;
} /* tst_list_del_pred */


/**
 * tst_list_for_each
 * @entry: the &tst_list_head_s to use as a loop counter
 * @head:  the head of the list
 *
 * Iterates over a list.
 */
#define tst_list_for_each(entry, head) \
	for (entry = (head)->next; entry != (head); entry = entry->next)


/**
 * tst_list_for_each_reverse
 * @entry: the &tst_list_head_s to use as a loop counter
 * @head:  the head of the list
 *
 * Iterates backwards over a list.
 */
#define tst_list_for_each_reverse(entry, head) \
	for (entry = (head)->prev; entry != (head); entry = entry->prev)


/**
 * tst_list_for_each_entry
 * @entry:  the type * to use as a loop counter
 * @head:   the head of the list
 * @member: the name of the tst_list_head_s within the struct
 *
 * Iterates over a list of typed entries.
 */
#define tst_list_for_each_entry(entry, head, member) \
	for (entry = tst_list_entry ((head)->next, \
	    typeof (* entry), member); \
	    &entry->member != (head); \
	    entry = tst_list_entry (entry->member.next, \
	    typeof (* entry), member))


/**
 * tst_list_for_each_entry_reverse
 * @entry:  the type * to use as a loop counter
 * @head:   the head of the list
 * @member: the name of the tst_list_head_s within the struct
 *
 * Iterates backwards over a list of typed entries.
 */
#define tst_list_for_each_entry_reverse(entry, head, member) \
	for (entry = tst_list_entry ((head)->prev, \
	    typeof (* entry), member); \
	    &entry->member != (head); \
	    entry = tst_list_entry (entry->member.prev, \
	    typeof (* entry), member))


/**
 * tst_list_destroy
 * @head:    the list to destroy
 * @destroy: destructor function to call on every entry
 *
 * Walks the list, removes all the entries from
 * the list and calls a destructor on them.
 */
static inline void
tst_list_destroy (tst_list_head_t head, tst_list_destroy_fn destroy)
{
	while (!tst_list_empty (head)) {
		tst_list_head_t entry = tst_list_del_succ (head);
		destroy (entry);
	}
} /* tst_list_destroy */


/**
 * tst_list_walk
 * @head:  the list to walk
 * @visit: the function to call on every entry
 * @data:  data for the visit function
 *
 * Walks the list and calls an visitor function on every entry.
 */
static inline void
tst_list_walk (tst_list_head_t head, tst_list_visit_fn visit, void * data)
{
	if (!tst_list_empty (head)) {
		tst_list_head_t entry;
		
		tst_list_for_each (entry, head) {
			visit (entry, data);
		}
	}
} /* tst_list_walk */


/**
 * tst_list_find
 * @head:  the list to walk
 * @match: the function to call on every entry
 * @data:  data for the match function
 *
 * Walks the list and calls a match function on every entry.
 * If a match is found, the matching entry is returned to the
 * caller, otherwise NULL is returned.
 */
static inline tst_list_head_t
tst_list_find (tst_list_head_t head, tst_list_match_fn match, void * data)
{
	if (!tst_list_empty (head)) {
		tst_list_head_t entry;
		
		tst_list_for_each (entry, head) {
			if (match (entry, data))
				return entry;
		}
	}
	
	return NULL;
} /* tst_list_find */


/**
 * __tst_list_get_nth_match
 *
 * Auxiliary function for tst_list_get_nth
 */
static inline int 
__tst_list_get_nth_match (tst_list_head_t item, int *cnt)
{
	if ((*cnt)-- == 0) {
		return 1;
	}
	
	return 0;
}

/**
 * tst_list_get_nth
 * @head: the list
 * @idx:  index of the entry
 *
 * Returns nth entry in the list. The entries are indexed from 0.
 * If the list is not so long NULL is returned.
 */
static inline tst_list_head_t
tst_list_get_nth (tst_list_head_t head, int idx)
{
  return tst_list_find (head,
      (tst_list_match_fn) __tst_list_get_nth_match, &idx);
}

#endif /* _TST_LIST_H_ */
