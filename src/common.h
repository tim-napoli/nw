#ifndef _common_h_
#define _common_h_

#if 0

typedef struct algo_cfg {
	int core_number;
} algo_cfg_t;


#endif

#define countof(_array)	(sizeof(_array) / sizeof(typeof(_array[0])))

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
typedef void (*algo_func_t)(const algo_arg_t*	args,
			    algo_res_t*		res);

typedef struct algo {
	char		name[64];
	char 		desc[256];
	algo_func_t	func;
} algo_t;

#endif

