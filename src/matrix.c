#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "common.h"
#include "matrix.h"

int matrix_init(matrix_t* m, int w, int h, int base_size) {
	m->v.v = mmap(NULL,
		      w * h * base_size,
		      PROT_READ | PROT_WRITE,
		      MAP_ANONYMOUS | MAP_PRIVATE,
		      -1,
		      0);
	if (m->v.v == MAP_FAILED) {
		return 1;
	}

	m->w = w;
	m->h = h;
	m->base_size = base_size;

	return 0;
}

void matrix_wipe(matrix_t* m) {
	munmap(m->v.v, m->w * m->h * m->base_size);
}

int matrix_diag_size(const matrix_t* m, int d) {
	int k = (d + 1) - m->w;
	if (k <= 0) {
		return min(d + 1, m->h);
	}
	else {
		return min(m->h - k, m->w);
	}
		  
}

/* TODO simplify this function */
int matrix_diag_offset(const matrix_t* m, int d) {
	int k = (d + 1) - m->w;

	if (m->w == m->h) {
		if (d <= m->w) {
			return (d * (d + 1)) / 2;
		}
		else {
			int rh = m->h - 1;
			int kh = m->h - k;
			int sum = (rh * (rh + 1)) / 2
				- (kh * (kh + 1)) / 2;
			return (m->w * (m->w + 1)) / 2
			     + sum;
		}
	}
	else if (m->w > m->h) {
		/*   0  1  2  3  4
		 *
		 * 0 f  f  f  m  m
		 *
		 * 1 f  f  m  m  l
		 *
		 * 2 f  m  m  l  l
		 *
		 */
		if (d < m->h) {
			return d * (d + 1) / 2;
		}
		else if (d < m->w) {
			int sup_d = m->h;
			int dec = d - sup_d;
			return (sup_d * (sup_d + 1)) / 2
			     + dec * m->h;
		}
		else {
			int sup_d = m->h;
			int dec = m->w - sup_d;
			int h = m->h - 1;
			int hk = m->h - k;
			int diff_sum = (h * (h + 1)) / 2
				     - (hk * (hk + 1)) / 2;
			return (sup_d * (sup_d + 1)) / 2
			     + dec * m->h
			     + diff_sum;
		}
	}
	else {
		/*   0  1  2
		 *
		 * 0 f  f  f
		 *
		 * 1 f  f  m
		 *
		 * 2 f  m  m
		 *
		 * 3 m  m  l
		 *
		 * 4 m  l  l
		 *
		 */
		if (d < m->w) {
			return d * (d + 1) / 2;
		}
		else if (d < m->h) {
			int sup_d = m->w;
			int dec = d - sup_d;
			return (sup_d * (sup_d + 1)) / 2
			     + dec * m->w;
		}
		else {
			k -= (m->h - m->w);
			int sup_d = m->w;
			int dec = m->h - sup_d;
			int w = m->w - 1;
			int wk = m->w - k;
			int diff_sum = (w * (w + 1)) / 2
				     - (wk * (wk + 1)) / 2;
			return (sup_d * (sup_d + 1)) / 2
			     + dec * m->w
			     + diff_sum;
		}
		return 0;
	}
}

int matrix_diag_y(const matrix_t* m, int diag) {
	if (diag < m->w) {
		return 0;
	}
	else {
		return diag - m->w + 1;
	}
}

int matrix_diag_x(const matrix_t* m, int diag) {
	if (diag < m->w) {
		return diag;
	}
	else {
		return m->w - 1;
	}
}

int matrix_coord_offset(const matrix_t* matrix, int x, int y) {
	int diag = x + y; 
	int diag_off = matrix_diag_offset(matrix, diag);
	int diag_x = matrix_diag_x(matrix, diag);
	int diag_y = matrix_diag_y(matrix, diag);
	int rel_x = abs(x - diag_x);
	int rel_y = abs(y - diag_y);
	return diag_off + min(rel_x, rel_y);
}

#ifdef TEST

#include <stdio.h>

static int __matrix_diag_offset_test(const matrix_t* m, int d) {
	int off = 0;

	for (int i = 0; i < d; i++) {
		off += matrix_diag_size(m, i);
	}

	return off;
}

int test_diag_offset(const matrix_t* m) {
	for (int d = 0; d < m->w + m->h - 1; d++) {
		int ref_off = __matrix_diag_offset_test(m, d);
		int off = matrix_diag_offset(m, d);

		if (ref_off != off) {
			printf("test_diag_offset error:\n"
			       "for diagonal %d, reference offset = %d\n"
			       "                 computed offset = %d\n",
			       d, ref_off, off);
			return 1;
		}
	}

	return 0;
}

int test_conversion_xy(void) {
	matrix_t m;
	matrix_init(&m, 4, 3, sizeof(int));

	int references[] = {
		0,  1,  3,  6,
		2,  4,  7,  9,
		5,  8, 10, 11
	};

	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 4; x++) {
			int off = matrix_coord_offset(&m, x, y);
			if (off != references[y * m.w + x]) {
				printf("error for %d %d: expected %d, got %d\n",
				       x, y, references[y * m.w + x], off);
				return 1;
			}
		}
	}

	matrix_wipe(&m);
	return 0;
}

int main(void) {
	matrix_t m_square;
	matrix_t m_width;
	matrix_t m_height;

	matrix_init(&m_square, 11, 11, sizeof(int));
	matrix_init(&m_width, 19, 7, sizeof(int));
	matrix_init(&m_height, 29, 7, sizeof(int));

	if (test_diag_offset(&m_square)) {
		printf("error with square matrix\n");
	}
	else {
		printf("offset of square matrix is OK\n");
	}
	if (test_diag_offset(&m_width)) {
		printf("error with width matrix\n");
	}
	else {
		printf("offset of height matrix is OK\n");
	}
	if (test_diag_offset(&m_height)) {
		printf("error with height matrix\n");
	}
	else {
		printf("offset of width matrix is OK\n");
	}

	matrix_wipe(&m_square);
	matrix_wipe(&m_width);
	matrix_wipe(&m_height);

	test_conversion_xy();

	return 0;
}

#endif

