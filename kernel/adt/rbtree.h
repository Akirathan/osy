/***
 * Red-black tree.
 *
 * The interface was originally inspired by Linux implementation of
 * red-black trees, and was later modified to be a bit more consistent.
 *
 * The implementation is based on description of red-black and binary
 * search trees found at Wikipedia:
 *  * http://en.wikipedia.org/wiki/Red-black_tree
 *  * http://en.wikipedia.org/wiki/Binary_search_tree
 *
 * Change Log:
 * 04/11/20 pallas created
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <include/shared.h>
#include <include/c.h>

#include <lib/debug.h>


/****************************************************************************\
| PUBLIC DEFS                                                                |
\****************************************************************************/

/** Color of the node in a red-black tree
 *
 */
enum rbcolor {
	RB_BLACK, RB_RED
};

/** Node of a red-black tree
 *
 */
struct rbnode {
	enum rbcolor color;
	struct rbnode *left;
	struct rbnode *right;
	struct rbnode *parent;
};

/** Red-black tree
 *
 */
struct rbtree {
	struct rbnode *root;
};

/** Various types of visitor/observer functions.
 *
 */
typedef int (* rbt_compare_fn) (struct rbnode *node, const void *data);


/****************************************************************************\
| PUBLIC FUNCTION MACROS                                                     |
\****************************************************************************/

/** Value used as invalid node
 *
 */
#define RBTREE_NULL  NULL


/** Static red-black tree initializer
 *
 * @param name Name of the rbtree variable
 *
 */
#define RBTREE_INIT(name) \
	{ \
		.color  = RB_RED, \
		.left   = RBTREE_NULL, \
		.right  = RBTREE_NULL, \
		.parent = RBTREE_NULL \
	}


/** Test whether the given node is a valid rbtree node
 *
 * Test whether the given node is a valid rbtree node. For that it must
 * differ from the value designated to identify invalid nodes.
 *
 * @param node Node to test
 *
 */
#define rbtree_is_node(node) \
	((node) != RBTREE_NULL)


/** Test whether the given node is the root node
 *
 * Test whether the given rbtree node is the root node. For that the
 * parent of the node must be invalid.
 *
 * @param node Node to test
 *
 */
#define rbtree_is_root(node) \
	(!rbtree_is_node ((node)->parent))


/** Test whether the given node is a leaf node
 *
 * Test whether the given rbtree node is a leaf node (not counting
 * the virtual sentinel nodes).
 *
 * @param node Node to test
 *
 */
#define rbtree_is_leaf(node) \
	((node)->left == (node)->right)


/** Test whether the given node is red
 *
 * Test whether the given rbtree node is red. For that it has to be
 * a valid node with color set to red.
 *
 * @param node Node to test
 *
 */
#define rbtree_is_red(node) \
	(rbtree_is_node (node) && (node)->color == RB_RED)


/** Test whether the given node is black
 *
 * Test whether the given rbtree node is black. For that it either
 * has to be the invalid (sentinel) node or a node with color set to black.
 *
 * @param node Node to test
 *
 */
#define rbtree_is_black(node) \
	(!rbtree_is_node (node) || (node)->color == RB_BLACK)


/** Test whether the given node has a left child
 *
 * @param node Node to test
 *
 */
#define rbtree_has_left(node) \
	(rbtree_is_node ((node)->left))


/** Test whether the given node has a right child
 *
 * @param node Node to test
 *
 */
#define rbtree_has_right(node) \
	(rbtree_is_node ((node)->right))


/** Test whether the given node has both children
 *
 * @param node Node to test
 *
 */
#define rbtree_has_both(node) \
	(rbtree_is_node ((node)->left) && rbtree_is_node ((node)->right))


/** Compute the offset of a member in a structure.
 *
 * @param type   Type of the container structure a member is embedded in
 * @param member Name of the member within the structure
 *
 * @return Offset of the member in the structure.
 *
 */
#define rbtree_offset_of(type, member) \
	((size_t) &((type *) 0)->member)


/** Cast a member of a structure to the containing structure
 *
 * @param ptr    Pointer to the member
 * @param type   Type of the container structure a member is embedded in
 * @param member Name of the member within the structure
 *
 * @return Container structure.
 *
 */
#define rbtree_container_of(ptr, type, member) \
	({ \
		const typeof (((type *) 0)->member) * __mptr = (ptr); \
		(type *) ((void *) __mptr - rbtree_offset_of (type, member)); \
	})


/** Get a typed structure from the given tree node
 *
 * @param ptr    Pointer to struct rbnode
 * @param type   Type of the struct the struct rbnode is embedded in
 * @param member Name of the field within the containing struct
 *
 */
#define rbtree_item(ptr, type, member) \
	rbtree_container_of (ptr, type, member)


/****************************************************************************\
| PRIVATE INLINE FUNCTIONS                                                   |
\****************************************************************************/

/** Initialize a node of a red-black tree
 *
 * @param node Node to initialize, it is assumed to be valid
 *
 */
static inline void __rbtree_init (struct rbnode *node)
{
	node->color = RB_RED;
	node->left = RBTREE_NULL;
	node->right = RBTREE_NULL;
	node->parent = RBTREE_NULL;
}


/** Walk a (sub)tree and return the leftmost node
 *
 * Walk a (sub)tree rooted in @root all the way to the left and return
 * the leftmost node as the first node in the sort order.
 *
 * @param root Root node of a (sub)tree, it is assumed to be valid
 *
 */
static inline struct rbnode *__rbtree_first (struct rbnode *root)
{
	while (rbtree_has_left (root))
		root = root->left;
	
	return root;
}


/** Walk a (sub)tree and return the rightmost node
 *
 * Walk a (sub)tree rooted in @root all the way to the right and return
 * the rightmost node as the last node in the sort order.
 *
 * @param root Root node of a (sub)tree, it is assumed to be valid
 *
 */
static inline struct rbnode *__rbtree_last (struct rbnode *root)
{
	while (rbtree_has_right (root))
		root = root->right;
	
	return root;
}


/** Return the successor
 *
 * Return the successor of tree node @node, in sort order.
 *
 * @param node Node of a tree, it is assumed to be valid
 *
 */
static inline struct rbnode *__rbtree_next (struct rbnode *node)
{
	/*
	 * If @node has a right child, take the leftmost
	 * node of of the right subtree.
	 */
	if (rbtree_has_right (node))
		return __rbtree_first (node->right);
	
	/*
	 * The @node did not have a right child, we are the the
	 * rightmost node of a subtree. The next node is the parent
	 * node of the subtree, therefore we have to walk the chain
	 * of parents up until we find a node that is a left child
	 * of its parent. The parent is the node we are looking for,
	 * and its child is the top of the subtree we were initially
	 * the rightmost node of.
	 */
	while (!rbtree_is_root (node) && node == node->parent->right)
		node = node->parent;
	
	/*
	 * We may return NULL if we got to the root, which signalizes
	 * that there is no 'next' node.
	 */
	return node->parent;
}


/** Return the predecessor
 *
 * Return the predecessor of tree node @node, in sort order.
 *
 * @param node Node of a tree, it is assumed to be valid
 *
 */
static inline struct rbnode *__rbtree_prev (struct rbnode *node)
{
	/*
	 * If @node has a left child, take the rightmost
	 * node of of the right subtree.
	 */
	if (rbtree_has_left (node))
		return __rbtree_last (node->left);
	
	/*
	 * The @node did not have a left child, we are the the
	 * leftmost node of a subtree. The next node is the parent
	 * node of the subtree, therefore we have to walk the chain
	 * of parents up until we find a node that is a right child
	 * of its parent. The parent is the node we are looking for,
	 * and its child is the top of the subtree we were initially
	 * the leftmost node of.
	 */
	while (!rbtree_is_root (node) && node == node->parent->left)
		node = node->parent;
	
	/*
	 * We may return NULL if we got to the root, which signalizes
	 * that there is no 'next' node.
	 */
	return node->parent;
}


/****************************************************************************\
| PUBLIC INLINE FUNCTIONS                                                    |
\****************************************************************************/

/** Initialize a node of a red-black tree
 *
 * @param node Node to initialize
 *
 */
static inline void rbtree_init (struct rbnode *node)
{
	assert (node != NULL);
	__rbtree_init (node);
}


/** Walk a (sub)tree and return the leftmost node
 *
 * Walk a (sub)tree rooted in @root all the way to the left and return
 * the leftmost node as the first node in the sort order.
 *
 * @param root Root node of a (sub)tree
 *
 */
static inline struct rbnode *rbtree_first (struct rbnode *root)
{
	assert (root != NULL);
	return __rbtree_first (root);
} /* rbtree_first */


/** Walk a (sub)tree and return the rightmost node
 *
 * Walk a (sub)tree rooted in @root all the way to the right and return
 * the rightmost node as the last node in the sort order.
 *
 * @param root Root node of a (sub)tree
 *
 */
static inline struct rbnode *rbtree_last (struct rbnode *root)
{
	assert (root != NULL);
	return __rbtree_last (root);
}


/** Return the successor
 *
 * Return the successor of tree node @node, in sort order.
 *
 * @param node Node of a tree
 *
 */
static inline struct rbnode *rbtree_next (struct rbnode *node)
{
	assert (rbtree_is_node (node));
	return __rbtree_next (node);
}


/** Return the predecessor
 *
 * Return the predecessor of tree node @node, in sort order.
 *
 * @param node Node of a tree
 *
 */
static inline struct rbnode *rbtree_prev (struct rbnode *node)
{
	assert (rbtree_is_node (node));
	return __rbtree_prev (node);
}


/** Walk the (sub)tree and call a compare function on each internal node
 *
 * Walk the (sub)tree and call a compare on each internal node. Depending
 * on the value returned by the compare function, the walk continues
 * either the left or the right subtree.
 *
 * @param root  (Sub)tree to search
 * @param match Callback function to call on every item
 * @param data  Data for the callback function
 *
 * @return The matching node if a match is found.
 * @return NULL otherwise.
 *
 */
static inline struct rbnode *rbtree_find (struct rbnode *root,
    rbt_compare_fn compare, const void *data)
{
	while (rbtree_is_node (root)) {
		int cmp = compare (root, data);
		
		/*
		 * compare returns -1 if key < root, 1 if key > root, and
		 * 0 if key == root.
		 */
		if (cmp < 0) {
			root = root->left;
		} else if (cmp > 0) {
			root = root->right;
		} else {
			return root;
		}
	}
	
	return NULL;
}


/****************************************************************************\
| PUBLIC FUNCTIONS                                                           |
\****************************************************************************/

/* Externals are commented with implementation. */
extern void rbtree_insert (struct rbtree *tree, struct rbnode *node,
    struct rbnode *parent, struct rbnode **clinkp);
extern void rbtree_delete (struct rbtree *tree, struct rbnode *node);
extern void rbtree_print (struct rbnode *root, const char *prefix);


#endif /* RBTREE_H_ */
