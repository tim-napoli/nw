#ifndef _alignment_h_
#define _alignment_h_

#include "common.h"
#include "matrix.h"

typedef struct alignment {
	char*	up;
	char*	down;
	size_t	size;
} alignment_t;


int alignment_init(alignment_t* al, size_t size);

void alignment_wipe(alignment_t* al);

int compute_alignments(const algo_arg_t* args,
		       const matrix_t* move_matrix,
		       alignment_t** alignments,
		       int bound);

void print_alignment(const alignment_t* al);

int score_alignment(const alignment_t* al);


#endif

