#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "common.h"
#include "matrix.h"

int matrix_init(matrix_t* m, int w, int h) {
	m->values = mmap(NULL,
			 w * h * sizeof(int),
			 PROT_READ | PROT_WRITE,
			 MAP_ANONYMOUS | MAP_PRIVATE,
			 -1,
			 0);
	if (m->values == MAP_FAILED) {
		return 1;
	}

	m->w = w;
	m->h = h;

	return 0;
}

void matrix_wipe(matrix_t* m) {
	munmap(m->values, m->w * m->h * sizeof(int));
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

/* Only for test !
 * This function, altough inefficient, is safe.
 */
static int __matrix_diag_offset_test(const matrix_t* m, int d) {
	int off = 0;

	for (int i = 0; i < d; i++) {
		off += matrix_diag_size(m, i);
	}

	return off;
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

#ifdef TEST

#include <stdio.h>

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

int main(void) {
	matrix_t m_square;
	matrix_t m_width;
	matrix_t m_height;

	matrix_init(&m_square, 11, 11);
	matrix_init(&m_width, 19, 7);
	matrix_init(&m_height, 29, 7);

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

	return 0;
}

#endif
