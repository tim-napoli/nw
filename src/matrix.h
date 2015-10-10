#ifndef _matrix_h_
#define _matrix_h_

/* Matrix are stored in memory by anti-diagonals (called "diagonals" in the next
 * of the code).
 * This fits better for Needleman-Wunsch processing.
 * The common way to use this matrix is to address it with a couple
 * (diagonal id, component offset in the diagonal).
 *
 *			   Matrix as it is
 *
 *			        0 1 2
 *
 * 			      0 a b c
 *
 * 			      1 d e f
 *
 * 			      2 g h i
 *
 *  	  Classical		    Diagonalized (grouped by diagonals)
 *
 *   0 1 2 3 4 5 6 7 8      		0   1 2   3 4 5   6 7   8
 *   a b c d e f g h i 			a   b d   c e g   f h   i
 *
 */
typedef struct matrix {
	int  w, h;
	int* values;
} matrix_t;

int matrix_init(matrix_t* m, int w, int h);

void matrix_wipe(matrix_t* m);

int matrix_diag_size(const matrix_t* m, int d);

int matrix_diag_offset(const matrix_t* m, int d);

typedef struct submatrix {
	int  diag_id;
	int* values;
} submatrix_t;

#endif

