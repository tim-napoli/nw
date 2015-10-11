#include <stdio.h>
#include "common.h"
#include "matrix.h"

static int __allocate_matrix(const algo_arg_t* args,
			     matrix_t* score_matrix,
			     matrix_t* move_matrix)
{
	if (matrix_init(score_matrix, args->len_a + 1, args->len_b + 1)) {
		printf("couldn't allocate score matrix\n");
		return 1;
	}
	if (matrix_init(move_matrix, args->len_a + 1, args->len_b + 1)) {
		matrix_wipe(score_matrix);
		printf("couldn't allocate move matrix\n");
		return 1;
	}
	return 0;
}

static void __init_matrix(const algo_arg_t* args,
			 matrix_t* score_matrix,
			 matrix_t* move_matrix)
{
	score_matrix->values[0] = 0;
	move_matrix->values[0] = MOVE_NONE;
	for (int x = 1; x < score_matrix->w; x++) {
		int off = matrix_coord_offset(score_matrix, x, 0);
		score_matrix->values[off] = -x;
		move_matrix->values[off] = MOVE_LEFT;
	}
	for (int y = 1; y < score_matrix->h; y++) {
		int off = matrix_coord_offset(score_matrix, 0, y);
		score_matrix->values[off] = -y;
		move_matrix->values[off] = MOVE_TOP;
	}
}

static void __compute_offsets(int w, int h, int d, int i,
			      int d1_off, int d2_off, int d3_off,
			      int* off_top, int* off_left, int* off_top_left)
{
	if (d < w) {
		*off_top	= d2_off + i - 1;
		*off_left	= d2_off + i;
		*off_top_left	= d1_off + i - 1;
	}
	else if (d == w) {
		*off_top	= d2_off + i;
		*off_left	= d2_off + i + 1;
		*off_top_left	= d1_off + i;
	}
	else {
		*off_top	= d2_off + i;
		*off_left	= d2_off + i + 1;
		*off_top_left	= d1_off + i + 1;
	}
}

static void __process_case(const algo_arg_t* args,
			   matrix_t* score_matrix,
			   matrix_t* move_matrix,
			   int d1_off, int d2_off, int d3_off,
			   int d, int i, int x, int y)
{
	/* First line and first column is already filled */
	if (x == 0 || y == 0) {
		return;
	}

	/* Compute offsets */
	int off_cur, off_top, off_left, off_top_left;
	off_cur  = d3_off + i;
	__compute_offsets(score_matrix->w, score_matrix->h, d, i,
			  d1_off, d2_off, d3_off,
			  &off_top, &off_left, &off_top_left);

	/* Values of neighbours */
	int values[3] = {
		score_matrix->values[off_top],
		score_matrix->values[off_left],
		score_matrix->values[off_top_left]
	};

	/* Potential values from move */
	int scores[3] = {
		values[0] - 1,
		values[1] - 1,
		values[2]
		+ ((args->seq_a[x - 1] == args->seq_b[y - 1]) ? 1 : -1)
	};

	int bests[3] = {
		(scores[0] >= scores[1] && scores[0] >= scores[2]) ? 1 : 0,
		(scores[1] >= scores[0] && scores[1] >= scores[2]) ? 1 : 0,
		(scores[2] >= scores[0] && scores[2] >= scores[1]) ? 1 : 0
	};
	int best = bests[0] ? 0 : bests[1] ? 1 : 2;

	score_matrix->values[off_cur] = scores[best];
	move_matrix->values[off_cur] = bests[0] * MOVE_TOP
				     | bests[1] * MOVE_LEFT
				     | bests[2] * MOVE_TOP_LEFT;
}

static void __process_diagonal(const algo_arg_t* args,
			       matrix_t* score_matrix,
			       matrix_t* move_matrix,
			       int diag)
{
	int d1_off = matrix_diag_offset(score_matrix, diag - 2);
	int d2_off = matrix_diag_offset(score_matrix, diag - 1);
	int d3_off = matrix_diag_offset(score_matrix, diag);
	int d3_size = matrix_diag_size(score_matrix, diag);
	int x = matrix_diag_x(score_matrix, diag);
	int y = matrix_diag_y(score_matrix, diag);

	for (int i = 0; i < d3_size; i++) {
		__process_case(args, score_matrix, move_matrix,
			       d1_off, d2_off, d3_off,
			       diag, i, x--, y++);
	}
}

static void __print_score_matrix(const algo_arg_t* args,
				 const matrix_t* score_matrix)
{
	printf("      - ");
	for (int x = 0; x < args->len_a; x++) {
		printf("%3c ", args->seq_a[x]);
	}
	printf("\n");
	
	for (int y = 0; y < args->len_b + 1; y++) {
		if (y == 0) {
			printf("  - ");
		}
		else {
			printf("%3c ", args->seq_b[y - 1]);
		}
		for (int x = 0; x < args->len_a + 1; x++) {
			int off = matrix_coord_offset(score_matrix, x, y);
			printf("%3d ", score_matrix->values[off]);
		}
		printf("\n");
	}

}

int nw(const algo_arg_t* args, algo_res_t* res) {
	matrix_t	score_matrix;
	matrix_t	move_matrix;

	/* Matrix initialisation */
	if (__allocate_matrix(args, &score_matrix, &move_matrix)) {
		return 1;
	}
	__init_matrix(args, &score_matrix, &move_matrix);

	for (int d = 2; d < args->len_a + args->len_b + 1; d++) {
		__process_diagonal(args, &score_matrix, &move_matrix, d);
	}

	__print_score_matrix(args, &score_matrix);

	matrix_wipe(&score_matrix);
	matrix_wipe(&move_matrix);

	return 0;
}

