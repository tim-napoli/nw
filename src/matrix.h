#ifndef _matrix_h_
#define _matrix_h_

typedef struct matrix {
	int  w, h;
	int* values;
} matrix_t;

#define matrix_diag_offset(_diag) \
	((_diag * (_diag + 1)) / 2)


typedef struct matrix_view {
	int  diag_id;
	int* values;
} matrix_view_t;

#endif

