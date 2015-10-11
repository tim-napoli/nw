#ifndef _common_h_
#define _common_h_

#include "matrix.h"

#if 0

typedef struct algo_cfg {
	int core_number;
} algo_cfg_t;

#endif

#define countof(_array)	(sizeof(_array) / sizeof(typeof(_array[0])))

#define max(_a, _b)	(((_a) > (_b)) ? (_a) : (_b))
#define min(_a, _b)	(((_a) < (_b)) ? (_a) : (_b))

/* Arguments of the Needleman-Wunsch algorithm.
 */
typedef struct algo_arg {
	char*	seq_a;
	char*	seq_b;
	int	len_a;
	int	len_b;
} algo_arg_t;

/* Result of the run of the algorithm
 */
typedef struct algo_res {
	int count;
	char**	al_x;
	char**	al_y;
	int*	len;
} algo_res_t;

/* Needleman-Wunsch Algorithm function type.
 */
typedef int (*algo_func_t)(const algo_arg_t*	args,
			   algo_res_t*		res,
			   matrix_t*		score_matrix,
			   matrix_t*		move_matrix);
typedef struct algo {
	char		name[64];
	char 		desc[256];
	algo_func_t	func;
} algo_t;

/* Moves values
 */
enum {
	MOVE_NONE	= 0,
	MOVE_TOP	= 1,
	MOVE_LEFT	= 2,
	MOVE_TOP_LEFT	= 4,
};



/* Algorithms prototypes
 */
int nw(const algo_arg_t* args, algo_res_t* res,
       matrix_t* score_matrix, matrix_t* move_matrix);
int nw_omp(const algo_arg_t* args, algo_res_t* res,
	   matrix_t* score_matrix, matrix_t* move_matrix);

/* Some cool functions
 */
void print_score_matrix(const algo_arg_t* args, const matrix_t* score_matrix);

#endif

