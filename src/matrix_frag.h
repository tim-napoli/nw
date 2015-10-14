#ifndef _matrix_frag_h_
#define _matrix_frag_h_

typedef struct matrix_frag
{
	/* data */
	//int *diags[3];
	int num_first_diag;
	int num_last_diag;
	int size_frag;
	int size_diag;

} matrix_frag_t;

void matrix_frag_init(matrix_frag_t *mf, int w , int h, int num_frag);






#endif