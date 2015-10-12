#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "alignment.h"

/* The alignment tree will follow the pathes from the last square of the move
 * matrix to the first square.
 * Doing that, deep-folding it from root to leaves will allow to build each
 * computed alignments.
 */
typedef struct altree {
	char		up[3];
	char		down[3];
	struct altree*	childs[3];	/* One child per direction */
	struct altree*	parent;		/* Could be useful for path
					 * reconstitution */
} altree_t;

/* Allocates a new tree node */
altree_t* altree_new(altree_t* parent) {
	altree_t* tree = malloc(sizeof(altree_t));
	if (tree == NULL) {
		printf("couldn't allocate alignment tree node\n");
		return NULL;
	}
	memset(tree, 0, sizeof(altree_t));
	tree->parent = parent;

	return tree;
}

/* Free node memory */
void altree_delete(altree_t* tree) {
	free(tree);
}

/* Free node and its childrens */
void altree_clear(altree_t* tree) {
	if (!tree) {
		return;
	}

	for (int i = 0; i < 3; i++) {
		altree_clear(tree->childs[i]);
	}
	altree_delete(tree);
}

/* Count number of leaves */
int altree_count_leaves(const altree_t* tree) {
	if (!tree) {
		return NULL;
	}

	if (!tree->childs[0] && !tree->childs[1] && !tree->childs[2]) {
		return 1;
	}

	int count = 0;
	for (int i = 0; i < 3; i++) {
		count += altree_count_leaves(tree->childs[i]);
	}

	return count;
}

/* Count tree max depth */
int altree_depth(const altree_t* tree) {
	if (!tree) {
		return 0;
	}

	int depth = altree_depth(tree->childs[0]);
	for (int i = 1; i < 3; i++) {
		int d = altree_depth(tree->childs[i]);
		if (d > depth) {
			depth = d;
		}
	}

	return 1 + depth;
}

/* Get tree leaves */
int altree_get_leaves(altree_t* tree, altree_t** leaves, int bound) {
	if (tree == NULL || bound == 0) {
		return 0;
	}

	int count = 0;
	for (int i = 0; i < 3; i++) {
		int atocount = altree_get_leaves(tree->childs[i], leaves, bound);
		leaves += atocount;
		count += atocount;
		bound -= atocount;
	}

	if (count == 0 && bound > 0) {
		*leaves = tree;
		count++;
	}

	return count;
}

static int _bound = 0;

/* Build a tree node */
static altree_t* __altree_build_node(const algo_arg_t* args,
				     const matrix_t* move_matrix,
				     int x, int y,
				     altree_t* parent)
{
	if (x < 0 || y < 0 || _bound == 0) {
		return NULL;
	}

	altree_t* node = altree_new(parent);
	if (node == NULL) {
		printf("error during node allocation\n");
		return NULL;
	}

	if (x == 0 && y == 0) {
		_bound--;
		return node;
	}

	size_t offset = matrix_coord_offset(move_matrix, x, y);
	if (move_matrix->v.c[offset] & MOVE_TOP) {
		node->up[0]	= '-';
		node->down[0]	= args->seq_b[y - 1];
		node->childs[0] = __altree_build_node(args, move_matrix,
						      x, y - 1,
						      node);
	}
	if (move_matrix->v.c[offset] & MOVE_LEFT) {
		node->up[1]	= args->seq_a[x - 1];
		node->down[1]	= '-';
		node->childs[1] = __altree_build_node(args, move_matrix,
						      x - 1, y,
						      node);
	}
	if (move_matrix->v.c[offset] & MOVE_TOP_LEFT) {
		node->up[2]	= args->seq_a[x - 1];
		node->down[2]	= args->seq_b[y - 1];
		node->childs[2] = __altree_build_node(args, move_matrix,
						      x - 1, y - 1,
						      node);
	}

	return node;
}

/* Build the tree */
altree_t* altree_build(const algo_arg_t* args,
		       const matrix_t* move_matrix)
{
	return __altree_build_node(args, move_matrix,
				   args->len_a, args->len_b,
				   NULL);
}

/* Allocates an alignment */
int alignment_init(alignment_t* al, size_t size) {
	al->up		= malloc(2 * size);
	if (!al->up) {
		printf("couldn't allocates alignment memory.\n");
		return 1;
	}
	al->down	= al->up + size;
	al->size	= size;
	return 0;
}

void alignment_wipe(alignment_t* al) {
	if (al->up) {
		free(al->up);
	}
}

/* Build an alignment giving a tree leaf */
static void __build_alignment(const altree_t* tree, alignment_t* al)
{
	int off = 0;
	while (tree->parent != NULL) {
		if (tree == tree->parent->childs[0]) {
			al->up[off] = tree->parent->up[0];
			al->down[off] = tree->parent->down[0];
		}
		else if (tree == tree->parent->childs[1]) {
			al->up[off] = tree->parent->up[1];
			al->down[off] = tree->parent->down[1];
		}
		else if (tree == tree->parent->childs[2]) {
			al->up[off] = tree->parent->up[2];
			al->down[off] = tree->parent->down[2];
		}
		tree = tree->parent;
		off++;
	}
	al->up[off] = '\0';
	al->down[off] = '\0';
}

/* Get alignments from the tree */
int altree_get_alignments(const algo_arg_t* args,
			  altree_t* tree,
			  alignment_t** alignments,
			  int bound)
{
	int depth = altree_depth(tree);
	if (bound <= 0) {
		bound = INT_MAX;
	}
	int nalignments = altree_count_leaves(tree);

	*alignments = malloc(nalignments * (sizeof(alignment_t)));
	if (!*alignments) {
		return -1;
	}
	memset(*alignments, 0, nalignments * sizeof(alignment_t));

	altree_t** leaves = malloc(nalignments * sizeof(altree_t*));
	if (!leaves) {
		printf("couldn't allocate node stack\n");
		goto error;
	}
	altree_get_leaves(tree, leaves, nalignments);

	for (int i = 0; i < nalignments; i++) {
		alignment_init(&(*alignments)[i], depth + 1);
		__build_alignment(leaves[i], (*alignments) + i);
	}

	free(leaves);

	return nalignments;

    error:
	for (int i = 0; i < nalignments; i++) {
		alignment_wipe((*alignments) + i);
	}
	free(*alignments);
	return -1;
}

int compute_alignments(const algo_arg_t* args,
		       const matrix_t* move_matrix,
		       alignment_t** alignments,
		       int bound)
{
	if (bound <= 0) {
		_bound = INT_MAX;
	}
	else {
		_bound = bound;
	}

	altree_t* tree = altree_build(args, move_matrix);
	if (tree == NULL) {
		printf("couldn't build alignment tree.\n");
		return -1;
	}

	int nalignments = altree_get_alignments(args, tree, alignments, bound);
	if (nalignments < 0) {
		printf("couldn't build alignments.\n");
		return -1;
	}

	altree_clear(tree);
	return nalignments;
}

void print_alignment(const alignment_t* al) {
	printf("%s\n%s\n", al->up, al->down);
}

