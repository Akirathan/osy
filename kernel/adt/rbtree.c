/***
 * Red-black tree.
 *
 * The interface was originally inspired by Linux implementation of
 * red-black trees, and was later modified to be a bit more consistent.
 *
 * The implementation is based on Linux and the description of red-black
 * and binary search trees found at Wikipedia:
 *  * http://en.wikipedia.org/wiki/Red-black_tree
 *  * http://en.wikipedia.org/wiki/Binary_search_tree
 *
 * Change Log:
 * 04/11/20 pallas created
 */

#include <adt/rbtree.h>


/****************************************************************************\
| PRIVATE CODE                                                               |
\****************************************************************************/

/** Move parent node
 *
 * Move the parent of @old node to the @new node and update the links in
 * the parent node originally pointing at @old node to point at the @new
 * node. If the @old node is root, the pointer to the root node in @tree is
 * updated to point at @new. If @new node is invalid, the @old node is severed
 * from its parent.
 *
 * NOTE: @tree must not be NULL and @old is assumed to be valid.
 *
 * @param tree Containing red-black tree
 * @param old  Old child the parent originally links to
 * @param new  New child the parent should link to
 *
 * @return a pointer to the parent node.
 *
 */
static struct rbnode *__rbtree_move_parent (struct rbtree *tree,
    struct rbnode *old, struct rbnode *new)
{
	struct rbnode *parent = old->parent;
	
	/*
	 * If the @new replacement is a valid node, set its parent.
	 */
	if (rbtree_is_node (new))
		new->parent = parent;
	
	/*
	 * If the parent is valid, update its child link.
	 */
	if (rbtree_is_node (parent)) {
		/*
		 * The @old node had a valid parent and was either its left
		 * or right child. Therefore the respective link from parent
		 * to @old must be replaced with link to @new.
		 */
		if (old == parent->left) {
			parent->left = new;
		} else {
			parent->right = new;
		}
		
	} else {
		/*
		 * Since @old was root and had no parent, @new is now root
		 * and the pointer to the root node has to be updated.
		 */
		tree->root = new;
	}
	
	return parent;
}


/** Move child links
 *
 * Move the child links from @old node to @new node and update the links
 * to the parent in the children. If @new is invalid, the children of
 * @old node (if any) will be orphaned.
 *
 * NOTE: The @old node is assumed to be valid.
 *
 * @param old Old child the parent originally links to
 * @param new New child the parent should link to
 *
 */
static void __rbtree_move_children (struct rbnode *old, struct rbnode *new)
{
	/*
	 * Copy the child links from @old node to @new.
	 */
	if (rbtree_is_node (new)) {
		new->left = old->left;
		new->right = old->right;
	}
	
	/*
	 * Update the parent links in left and right child of @old.
	 */
	if (rbtree_has_left (old))
		old->left->parent = new;
	
	if (rbtree_has_right (old))
		old->right->parent = new;
}


/****************************************************************************\
| PROTECTED CODE                                                             |
\****************************************************************************/

/** Left rotation around node @pivot
 *
 * Perform a left rotation around node @pivot. That means that the right
 * child of @pivot node becomes @pivot's parent, while the child's left
 * subtree becomes @pivot's right subtree. The operation preserves the
 * in-order key ordering.
 *
 *         {R}                (P)
 *        /   \     left     /   \
 *     (P)     [c]  <---  [a]     {R}
 *    /   \                      /   \
 * [a]     [b]                [b]     [c]
 *
 * NOTE: @tree must not be NULL and both @pivot and its right child are
 * assumed to be valid.
 *
 * @param tree  Containing red-black tree
 * @param pivot Node to rotate around
 *
 */
static void __rbtree_rotate_left (struct rbtree *tree, struct rbnode *pivot)
{
	struct rbnode *child = pivot->right;
	
	/*
	 * Replace @pivot's right child with the left
	 * subtree of the that child.
	 */
	pivot->right = child->left;
	child->left = pivot;
	
	/*
	 * Update the parent of the moved subtree.
	 */
	if (rbtree_has_right (pivot))
		pivot->right->parent = pivot;
	
	/*
	 * Move the parent from @pivot to @child and make @child the
	 * new parent of @pivot.
	 */
	__rbtree_move_parent (tree, pivot, child);
	pivot->parent = child;
}


/** Right rotation around node @pivot
 *
 * Perform a right rotation around node @pivot. That means that the left
 * child of @pivot node becomes @pivot's parent, while the child's right
 * subtree becomes @pivot's left subtree. The operation preserves the
 * in-order key ordering.
 *
 *         (P)                 {L}
 *        /   \     right     /   \
 *     {L}     [c]  ---->  [a]     (P)
 *    /   \                       /   \
 * [a]     [b]                 [b]     [c]
 *
 * NOTE: @tree must not be NULL and both @pivot and its left child are
 * assumed to be valid.
 *
 * @param tree  Containing red-black tree
 * @param pivot Node to rotate around
 *
 */
static void __rbtree_rotate_right (struct rbtree *tree, struct rbnode *pivot)
{
	struct rbnode *child = pivot->left;
	
	/*
	 * Replace @pivot's left child with the right
	 * subtree of the that child.
	 */
	pivot->left = child->right;
	child->right = pivot;
	
	/*
	 * Update the parent of the moved subtree.
	 */
	if (rbtree_has_left (pivot))
		pivot->left->parent = pivot;
	
	/*
	 * Move the parent from @pivot to @child and make @child the
	 * new parent of @pivot.
	 */
	__rbtree_move_parent (tree, pivot, child);
	pivot->parent = child;
}


/** Tree rotation
 *
 * @param tree  Containing red-black tree
 * @param pivot Node to rotate around
 * @param left  Flag to indicate left rotation
 *
 */
static void __rbtree_rotate (struct rbtree *tree, struct rbnode *pivot,
    bool left)
{
	if (left)
		__rbtree_rotate_left (tree, pivot);
	else
		__rbtree_rotate_right (tree, pivot);
}


/*
 * Red-black tree properties:
 *
 * 1) [blackroot]   The root is black
 * 2) [blackleaves] All leaves are black
 * 3) [reddblack]   Both children of a red node are black.
 * 4) [blackcount]  Paths from each leaf up to the root
 *                  contains the same number of black nodes
 */


/** Rebalance the tree after inserting a new red node
 *
 * Rebalance the tree after inserting a new red node into the tree to
 * satisfy the properties of red-black tree.
 *
 * The rebalancing takes O(log(N)) color changes and O(1) tree rotations (at
 * most two for insertion). Thus the complexity of insertion is O(log(N)),
 * which is the same as for AVL trees, but the number of (costly) rotations
 * is O(1) compared to O(log(N)) for AVL trees.
 *
 * NOTE: @tree must not be NULL and @node is assumed to be valid.
 *
 * @param tree Containing red-black tree
 * @param node Newly inserted node colored red
 *
 */
static void __rbtree_rebalance_insert (struct rbtree *tree, struct rbnode *node)
{
	/*
	 * For root nodes, the parent cannot be red, because first it would
	 * have to exist, which it cannot in case of root @node.
	 *
	 * For non-root nodes, if the parent is black, [redblack] is not
	 * violated, but [blackcount] may be violated, because the new node
	 * has two black children. Yet since the (new) node is red and it
	 * replaced a (virtual) black leaf, the paths through each of its
	 * children have the same number of black nodes as the path through
	 * the (virtual) leaf that has been replaced.
	 *
	 * Therefore we only need to handle the case when the parent of the
	 * new node is red, which violates [redblack].
	 */
	while (rbtree_is_red (node->parent)) {
		bool nleft;  /* @node is left child */
		bool pleft;  /* @parent is left child */
		
		struct rbnode *parent;   /* Parent of @node */
		struct rbnode *gparent;  /* Parent of @parent */
		struct rbnode *uncle;    /* Child of @gparent */
		
		parent = node->parent;
		
		/*
		 * The @parent of the (new) node is red, therefore the node
		 * also has a grandparent @gparent, because if @parent were
		 * root, it would be black. The node also has an @uncle,
		 * which may be a (virtual) black leaf.
		 */
		gparent = parent->parent;
		uncle = (gparent->left == parent) ?
		    gparent->right : gparent->left;
		
		/*
		 * If the @uncle is red, then both @parent and @uncle are red,
		 * and we can paint them black and paint the grandparent red.
		 * The new (red) @node now has a black @parent, so that the
		 * new @node does not violate [redblack] anymore. Painting
		 * the grandparent red assured that the number of black nodes
		 * along the path is not changed, since all paths must pass
		 * through grandparent. Yet painting the grandparent red may
		 * have violated [redblack] with respect to its own parent,
		 * so we have to repeat the process recursively with the
		 * grandparent in the role of the new @node.
		 */
		if (rbtree_is_red (uncle)) {
			uncle->color = RB_BLACK;
			parent->color = RB_BLACK;
			gparent->color = RB_RED;
			
			node = gparent;
			continue;
		}
		
		/*
		 * The @uncle is black and @gparent is black as well, because
		 * otherwise @parent could not be red. Now we have 2 cases
		 * depending on which child of @gparent is @parent and which
		 * child of @parent is @node.
		 */
		nleft = (parent->left == node);
		pleft = (gparent->left == parent);
		
		/*
		 * If the new @node is an inner node instead of boundary node
		 * of the subtree rooted in @gparent, we swap its role with
		 * its @parent using rotation round the parent, which moves it
		 * to the boundary of the tree.
		 *
		 * This is the case when the @parent is left child of @gparent
		 * and @node is right child of @parent, or if the @parent is
		 * right child of @gparent and @node is left child of @parent.
		 *
		 * Since both @node and @parent are red, this change does not
		 * violate [blackcount].
		 */
		if (nleft != pleft) {
			__rbtree_rotate (tree, parent, pleft);
			
			/*
			 * Update @node and @parent to reflect the new state.
			 */
			node = parent;
			parent = node->parent;
		}
		
		/*
		 * If the new @node is a boundary node of the subtree rooted
		 * in @gparent, we rotate the tree around @gparent to move
		 * the @parent in @gparent's position and the @gparent in
		 * the @uncle's position.
		 *
		 * The relation ship between @node and @parent is not changed,
		 * the relationship between @parent and @gparent is swapped
		 * but not needed anymore.
		 */
		__rbtree_rotate (tree, gparent, !pleft);
		
		/*
		 * Since @node and @parent still violate [redblack], we switch
		 * colors of @parent and @gparent (which is black) so that
		 * now the @node is red, @parent is black and @gparent is red.
		 * This fixes the violation of [redblack]. Since any paths
		 * that went through @node or @parent also went through black
		 * @gparent, they now go through black @parent and may lead to
		 * red @node or red @gparent, which satisfies [blackcount].
		 */
		gparent->color = RB_RED;
		parent->color = RB_BLACK;
	}
	
	/*
	 * Always paint the root of the tree black to satisfy [blackroot].
	 * This could violate [blackcount], but in case of root node we only
	 * add one black node to every path in the tree, so [blackcount]
	 * remains satisfied.
	 */
	tree->root->color = RB_BLACK;
}


/** Rebalance a subtree after deleting a node
 *
 * Rebalance a subtree after deleting a node from the tree to satisfy
 * the properties of red-black tree.
 *
 * The rebalancing takes O(log(N)) color changes and O(1) tree rotations (at
 * most three for deletion). Thus the complexity of deletion is O(log(N)),
 * which is the same as for AVL trees, but the number of (costly) rotations
 * is O(1) compared to O(log(N)) for AVL trees.
 *
 * Note: @tree must not be NULL and @parent is assumed to be valid.
 *
 * @param tree   Containing red-black tree
 * @param parent Parent of the (black) node that was removed
 * @param node   Child of the (black) node that was removed
 *
 */
static void __rbtree_rebalance_delete (struct rbtree *tree,
    struct rbnode *parent, struct rbnode *node)
{
	/*
	 * We have deleted a black node whose child was @node and whose
	 * parent was @parent, who is now the parent of @node. The @parent
	 * node is initially the root of the subtree that needs to be
	 * rebalanced.
	 *
	 * If the node is red, we will just repaint it black and be done,
	 * which is the simple case. If the node is black but is also the
	 * root of the whole tree, we don't need to do anything else.
	 *
	 * Other cases are more complex...
	 */
	while ((rbtree_is_black (node)) && (node != tree->root)) {
		struct rbnode * sibling;  /* Sibling of @node */
		bool nleft;               /* @node is left child */
		
		/*
		 * Since @node is not the root of the tree, it has a valid
		 * @parent. We determine which child of @parent the @node is
		 * and obtain a reference to its @sibling.
		 */
		if (node == parent->left) {
			sibling= parent->right;
			nleft = true;
		} else {
			sibling = parent->left;
			nleft = false;
		}
		
		/*
		 * If the @sibling is red, the @parent must be black. We then
		 * rotate around the @parent and reverse the colors of @parent
		 * (black) and @sibling (red).
		 *
		 * This does not change the number of black nodes in all paths
		 * going through the subtree, but we will obtain a tree where
		 * the (black) @node has red @parent and black @sibling.
		 */
		if (rbtree_is_red (sibling)) {
			__rbtree_rotate (tree, parent, nleft);
			
			parent->color = RB_RED;
			sibling->color = RB_BLACK;
			sibling = nleft ? parent->right : parent->left;
		}
		
		/*
		 * We now have a black @node with black @sibling (either
		 * originally, or after the above rotation), and either red
		 * or black @parent.
		 *
		 * If both children of the @sibling are black, we can repaint
		 * the sibling red and must ensure that @parent is black, so
		 * that we don't violate [redblack].
		 *
		 * If the @parent was red, this did not change the number of
		 * black nodes going through the @sibling, and adds one black
		 * node to paths going through @node, which makes up for the
		 * deleted node.
		 *
		 * If the @parent was black, all paths going through @sibling
		 * will have one less black node, which makes up for the one
		 * black node removed from between @parent and @node. However,
		 * the paths now going through @parent are one black node
		 * shorter than paths not going through @parent, so we have
		 * to restart the balancing in @parent.
		 */
		if ((rbtree_is_black (sibling->left)) &&
		    (rbtree_is_black (sibling->right))) {
			sibling->color = RB_RED;
			
			/*
			 * If the @parent was black, we should restart the
			 * rebalancing, if it was red, we should repaint it
			 * black and stop. When @node becomes @parent and is
			 * red, the rebalancing will stop and the node will
			 * be repainted black.
			 */
			node = parent;
			parent = node->parent;
			
		} else {
			struct rbnode *inner;
			
			/*
			 * The @sibling has at least one red child. Depending
			 * on the original position of @node with respect to
			 * its @parent, the @sibling has an @inner and outer
			 * children.
			 */
			inner = nleft ? sibling->right : sibling->left;
			
			/*
			 * If the @inner child is black, then there must be
			 * red @outer child. We rotate around the sibling to
			 * make the @outer child the parent of @sibling.
			 *
			 * The @outer child has thus become the new sibling
			 * to @node. We then swap the colors of @sibling and
			 * its new parent.
			 *
			 * The old @sibling becomes the @inner child of the
			 * new sibling.
			 */
			if (rbtree_is_black (inner)) {
				struct rbnode *outer;
				
				outer = nleft ?
				    sibling->left : sibling->right;
				
				assert (rbtree_is_red (outer));
				__rbtree_rotate (tree, sibling, !nleft);
				
				outer->color = RB_BLACK;
				sibling->color = RB_RED;
				
				inner = sibling;
				sibling = outer;
			}
			
			/*
			 * In the last case, @node has a black @sibling with
			 * red @inner child. We rotate around @parent in the
			 * same direction as the direction from @parent to
			 * @node.
			 *
			 * The @sibling becomes the new parent of @parent. We
			 * copy the color from @parent to @sibling and repaint
			 * the @parent and @inner child to black (to avoid
			 * violating [redblack] if the @parent was originally
			 * red).
			 */
			assert (rbtree_is_red (inner));
			__rbtree_rotate (tree, parent, nleft);
			
			sibling->color = parent->color;
			parent->color = RB_BLACK;
			inner->color = RB_BLACK;
			
			/*
			 * Make sure to repaint the root of the tree black
			 * after rebalancing.
			 */
			node = tree->root;
			break;
		} /* Sibling had at least one red child */
		
	}
	
	/*
	 * If the child of the deleted black node was red, repaint it to
	 * black, which covers the (initial) simple case.
	 *
	 * If the child was black, the @node here will be the result of
	 * rebalancing iterations (@parent in the first iterations) that
	 * has to be repainted black.
	 */
	if (rbtree_is_node (node))
		node->color = RB_BLACK;
}


/** Replace @old node with @new node
 *
 * Replace @old node with @new node, without the need to rebalance
 * the tree.
 *
 * The @new node copies the color and links of from the @old node. If the
 * @old node was root, the root node of the @tree is updated as well. If
 * @new is invalid, the @old node is simply deleted from the tree and if
 * it had children, these will be orphaned.
 *
 * Note: @tree must not be NULL and @old is assumed to be valid.
 *
 * @param tree Containing red-black tree
 * @param old  Node in tree to replace
 * @param new  Node to replace the old node with
 *
 */
static void __rbtree_replace (struct rbtree *tree, struct rbnode *old,
    struct rbnode *new)
{
	/*
	 * Copy the color from @old node to @new.
	 */
	if (rbtree_is_node (new))
		new->color = old->color;
	
	/*
	 * Copy the child links from the @old node to the @new node and
	 * and update the parent links in the children.
	 */
	__rbtree_move_children (old, new);
	
	/*
	 * Move the parent from the @old node to the @new node and update
	 * the child links in the parent.
	 */
	__rbtree_move_parent (tree, old, new);
}


/** Insert a new red @node into tree
 *
 * Insert a new red @node into tree as a child of node @parent while
 * maintaining the properties of a red-black tree.
 *
 * The @parent may be invalid if we are inserting a root node in the tree.
 * The child link pointer @clinkp either points at one of the children links
 * in @parent, or at the root link in @tree.
 *
 * NOTE: Neither @tree nor @clinkp may be NULL. The new @node is assumed to be
 * valid and @parent may be invalid if we are creating a root node.
 *
 * @param tree   Containing red-black tree
 * @param node   Node to insert
 * @param parent Node to serve as parent to @node
 * @param clinkp Pointer to the appropriate child link in @parent
 *
 */
static void __rbtree_insert (struct rbtree *tree, struct rbnode *node,
    struct rbnode *parent, struct rbnode **clinkp)
{
	/*
	 * Link the node in the tree.
	 */
	node->color = RB_RED;
	node->left = NULL;
	node->right = NULL;
	
	node->parent = parent;
	(*clinkp) = node;
	
	/*
	 * Rebalance the tree.
	 */
	__rbtree_rebalance_insert (tree, node);
}


/** Delete a node @node from a red-black tree
 *
 * Delete a node @node from a red-black tree while maintaining the
 * properties of a red-black tree.
 *
 * Note: @tree must not be NULL and @node is assumed to be valid.
 *
 * @param tree Containing red-black tree
 * @param node Node to delete from tree
 *
 */
static void __rbtree_delete (struct rbtree *tree, struct rbnode *node)
{
	enum rbcolor color;     /* Color of the actually removed node */
	struct rbnode *child;   /* Child of the actually removed node */
	struct rbnode *parent;  /* Parent of the actually removed node */
	
	/*
	 * When deleting a @node from binary tree, we have to find a node
	 * from @node's subtree that will replace the deleted node.
	 *
	 * The situation is rather easy if the deleted @node has no or only
	 * a signle child. If a node has both children, the situation becomes
	 * more complex.
	 */
	if (!rbtree_has_both (node)) {
		/*
		 * The deleted @node has at most one child, therefore we
		 * can replace the child link in @node's parent with the
		 * child or reset the link to NULL.
		 *
		 * If the node has no children, it is always replaced by a
		 * (virtual) black node.
		 *
		 */
		child = rbtree_has_left (node) ? node->left : node->right;
		parent = __rbtree_move_parent (tree, node, child);
		color = node->color;
		
	} else {
		struct rbnode *subst;  /* Substitute node */
		
		/*
		 * The deleted @node has both children, therefore we have to
		 * find a node with a most one child which will substitute the
		 * deleted node. The substitute can be either the rightmost
		 * node from the left subtree, or the leftmost node from the
		 * right subtree. Either is guaranteed to have at most one
		 * child.
		 *
		 * We select the leftmost node from the right subtree, which
		 * can at most have a right child (otherwise it would not be
		 * leftmost).
		 *
		 *     (N)                  [N]
		 *    /   \                /   \
		 * [a]     [d]   --->   [a]     [d]
		 *        /   \                /   \
		 *     [b]     [e]          (b)     [e]
		 *        \                    \
		 *         [c]                  {c}
		 */
		subst = rbtree_first (node->right);
		child = subst->right;
		
		/*
		 * We move the child under the parent of @subst, which will
		 * remove @subst from the tree, so we must remember its color.
		 *
		 *     [N]              (b) [N]
		 *    /   \                /   \
		 * [a]     [d]   --->   [a]     [d]
		 *        /   \                /   \
		 *     (b)     [e]          {c}     [e]
		 *        \
		 *         {c}
		 */
		parent = __rbtree_move_parent (tree, subst, child);
		color = subst->color;
		
		/*
		 * Then we replace @node with the already removed substitute
		 * node @subst, which will copy the color and child and parent
		 * links from @node and will also update the child links in
		 * @node's parent.
		 *
		 * (b) {N}                  (b) {N}
		 *    /   \                /   \
		 * [a]     [d]   --->   [a]     [d]
		 *        /   \                /   \
		 *     [c]     [e]          [c]     [e]
		 */
		__rbtree_replace (tree, node, subst);
		
		/*
		 * The @parent node should point at the parent of the really
		 * deleted node, i.e. @subst. If @subst was a direct child of
		 * @node though, @parent will point at @node which has been
		 * replaced by @subst, therefore @parent has to point at the
		 * replaced node @subst.
		 */
		if (parent == node)
			parent = subst;
	}
	
	/*
	 * Rebalance the tree if we have deleted a black node, because that
	 * may have violated [blackcount].
	 *
	 * If the deleted @node had two children, another node with at most
	 * one child was selected to be deleted instead and the original @node
	 * was replaced by the deleted node.
	 *
	 * If a red node was really deleted, it had at most one (black) child
	 * and was either replaced by the child or removed, which does not
	 * change [blackcount].
	 */
	if (color == RB_BLACK)
		__rbtree_rebalance_delete (tree, parent, child);
}


/****************************************************************************\
| PUBLIC CODE                                                                |
\****************************************************************************/

#if 0

// TODO: This is a sample code showing how to pretty-print
//       the contents of a red-black tree

/** Print a list representation of the tree
 *
 * @param root   Root of a tree/subtree to print
 * @param prefix Prefix to print before the list
 *
 */
void rbtree_print (struct rbnode *root, const char *prefix)
{
	struct vma *vma;
	
	if (prefix != NULL)
		printk ("%s", prefix);
	
	if (root != NULL) {
		vma = rbtree_item (root, struct vma, vma_rbnode);
		
		printk ("(");
		rbtree_print (root->left, NULL);
		
		printk ("<- %c%x,%x/%c%c ->",
		    prefix != NULL ? '{' : '[',
		    vma->vma_start, vma->vma_end,
		    root->color == RB_BLACK ? 'B' :
		    root->color == RB_RED ? 'R' : 'U',
		    prefix != NULL ? '}' : ']');
		
		rbtree_print (root->right, NULL);
		printk (")");
	} else
		printk ("N");
	
	if (prefix != NULL)
		printk ("\n");
}

#endif


/** Insert a new red @node into tree
 *
 * Insert a new red @node into tree as a child of node @parent while
 * maintaining the properties of a red-black tree.
 *
 * @param tree   Containing red-black tree
 * @param node   Node to insert
 * @param parent Node to serve as parent to @node
 * @param clinkp Pointer to the appropriate child link in @parent
 *
 */
void rbtree_insert (struct rbtree *tree, struct rbnode *node,
    struct rbnode *parent, struct rbnode **clinkp)
{
	assert (tree != NULL);
	assert (clinkp != NULL);
	assert (rbtree_is_node (node));
	
	__rbtree_insert (tree, node, parent, clinkp);
}


/** Delete a node @node from a red-black tree
 *
 * Delete a node @node from a red-black tree while maintaining the
 * properties of a red-black tree.
 *
 * @param tree Containing red-black tree
 * @param node Node to delete from tree
 *
 */
void rbtree_delete (struct rbtree *tree, struct rbnode *node)
{
	assert (tree != NULL);
	assert (rbtree_is_node (node));
	
	__rbtree_delete (tree, node);
}
