#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "matrix.h"

static void __init_matrix(const algo_arg_t* args,
			 matrix_t* move_matrix)
{
	move_matrix->v.c[0] = MOVE_NONE;
	for (int x = 1; x < move_matrix->w; x++) {
		size_t off = matrix_coord_offset(move_matrix, x, 0);
		move_matrix->v.c[off] = MOVE_LEFT;
	}
	for (int y = 1; y < move_matrix->h; y++) {
		size_t off = matrix_coord_offset(move_matrix, 0, y);
		move_matrix->v.c[off] = MOVE_TOP;
	}
}

static void __compute_offsets(int w, int h, int d, int i,
			      size_t d1_off,
			      size_t d2_off,
			      size_t d3_off,
			      size_t* off_top,
			      size_t* off_left,
			      size_t* off_top_left)
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
			   int** wscores,
			   matrix_t* move_matrix,
			   size_t d1_off, size_t d2_off, size_t d3_off,
			   int d, int i, int x, int y)
{
	/* Compute offsets */
	size_t off_cur, off_top, off_left, off_top_left;
	off_cur  = d3_off + i;
	__compute_offsets(move_matrix->w, move_matrix->h, d, i,
			  d1_off, d2_off, d3_off,
			  &off_top, &off_left, &off_top_left);

	/* First line and first column is already filled */
	if (x == 0 || y == 0) {
		/* XXX be carful about concurency here */
		if (x == 0) {
			wscores[2][off_cur - d3_off] = -y;
		}
		else if (y == 0) {
			wscores[2][off_cur - d3_off] = -x;
		}
		return;
	}

	/* Values of neighbours */
	int values[3] = {
		wscores[1][off_top - d2_off],
		wscores[1][off_left - d2_off],
		wscores[0][off_top_left - d1_off]
	};

	/* Potential values from move */
	int scores[3] = {
		values[0] - 1,
		values[1] - 1,
		values[2]
		+ ((args->seq_a[x - 1] == args->seq_b[y - 1]) ? 1 : -1)
	};

	/* What is the best score ? */
	int bests[3] = {
		(scores[0] >= scores[1] && scores[0] >= scores[2]) ? 1 : 0,
		(scores[1] >= scores[0] && scores[1] >= scores[2]) ? 1 : 0,
		(scores[2] >= scores[0] && scores[2] >= scores[1]) ? 1 : 0
	};
	int best = bests[0] ? 0 : bests[1] ? 1 : 2;

	/* Fill the result */
	wscores[2][off_cur - d3_off] = scores[best];
	move_matrix->v.c[off_cur]   = bests[0] * MOVE_TOP
				    | bests[1] * MOVE_LEFT
				    | bests[2] * MOVE_TOP_LEFT;
}

static void __process_diagonal(const algo_arg_t* args,
			       int** wscores,
			       matrix_t* move_matrix,
			       int diag)
{
	/* Diagonal offsets */
	size_t d1_off = matrix_diag_offset(move_matrix, diag - 2);
	size_t d2_off = matrix_diag_offset(move_matrix, diag - 1);
	size_t d3_off = matrix_diag_offset(move_matrix, diag);

	/* Current diagonal size */
	int d3_size = matrix_diag_size(move_matrix, diag);

	/* Coordinates of the current diagonal first case */
	int x = matrix_diag_x(move_matrix, diag);
	int y = matrix_diag_y(move_matrix, diag);

	for (int i = 0; i < d3_size; i++) {
		int dx = x - i;
		int dy = y + i;
		__process_case(args, wscores, move_matrix,
			       d1_off, d2_off, d3_off,
			       diag, i, dx, dy);
	}

	/* Move score diagonales */
	int* tmp = wscores[0];
	wscores[0] = wscores[1];
	wscores[1] = wscores[2];
	wscores[2] = tmp;
}

static void __process_diagonal_omp(const algo_arg_t* args,
				   int** wscores,
				   matrix_t* move_matrix,
				   int diag)
{
	/* Diagonal offsets */
	size_t d1_off = matrix_diag_offset(move_matrix, diag - 2);
	size_t d2_off = matrix_diag_offset(move_matrix, diag - 1);
	size_t d3_off = matrix_diag_offset(move_matrix, diag);

	/* Current diagonal size */
	int d3_size = matrix_diag_size(move_matrix, diag);

	/* Coordinates of the current diagonal first case */
	int x = matrix_diag_x(move_matrix, diag);
	int y = matrix_diag_y(move_matrix, diag);

	#pragma omp parallel for
	for (int i = 0; i < d3_size; i++) {
		int dx = x - i;
		int dy = y + i;
		__process_case(args, wscores, move_matrix,
			       d1_off, d2_off, d3_off,
			       diag, i, dx, dy);
	}

	/* Move score diagonales */
	int* tmp = wscores[0];
	wscores[0] = wscores[1];
	wscores[1] = wscores[2];
	wscores[2] = tmp;
}

int nw(const algo_arg_t* args, algo_res_t* res,
       matrix_t* move_matrix)
{
	int* score_buf = NULL;

	/* Matrix initialisation */
	__init_matrix(args, move_matrix);

	/* Initialize score windows */
	size_t size_win = args->len_a + args->len_b + 2;
	score_buf = malloc(3 * size_win * sizeof(int));
	if (!score_buf) {
		printf("couldn't allocates score windows buffer\n");
		return 1;
	}

	/* Windows initialisation */
	int* wscores[3] = {
		score_buf,
		score_buf + size_win,
		score_buf + 2 * size_win
	};
	wscores[0][0] = 0;
	wscores[1][0] = -1;
	wscores[1][1] = -1;

	size_t total_size = (args->len_a + 1) * (size_t) (args->len_b + 1);
	size_t current = 3;
	for (int d = 2; d < args->len_a + args->len_b + 1; d++) {
		__process_diagonal(args, wscores, move_matrix, d);

		current += matrix_diag_size(move_matrix, d);
		if (d % 10 == 0) {
			VERBOSE_FMT("progression: %f%%\r",
				    current * 100 / (float) total_size);
		}
	}
	VERBOSE("\n");

	free(score_buf);

	return 0;
}

int nw_omp(const algo_arg_t* args, algo_res_t* res,
       matrix_t* move_matrix)
{
	int* score_buf = NULL;

	/* Matrix initialisation */
	__init_matrix(args, move_matrix);

	/* Initialize score windows */
	size_t size_win = args->len_a + args->len_b + 2;
	score_buf = malloc(3 * size_win * sizeof(int));
	if (!score_buf) {
		printf("couldn't allocates score windows buffer\n");
		return 1;
	}

	/* Windows initialisation */
	int* wscores[3] = {
		score_buf,
		score_buf + size_win,
		score_buf + 2 * size_win
	};
	wscores[0][0] = 0;
	wscores[1][0] = -1;
	wscores[1][1] = -1;

	size_t total_size = (args->len_a + 1) * (size_t) (args->len_b + 1);
	size_t current = 3;
	for (int d = 2; d < args->len_a + args->len_b + 1; d++) {
		__process_diagonal_omp(args, wscores, move_matrix, d);

		current += matrix_diag_size(move_matrix, d);
		if (d % 10 == 0) {
			VERBOSE_FMT("progression: %f%%\r",
				    current * 100 / (float) total_size);
		}
	}
	VERBOSE("\n");

	free(score_buf);

	return 0;
}

void print_move_matrix(const algo_arg_t* args,
		       const matrix_t* move_matrix)
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
			int off = matrix_coord_offset(move_matrix, x, y);
			printf("%3x ", move_matrix->v.c[off]);
		}
		printf("\n");
	}

}


