#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>

int main(int argc, char** argv)
{
	size_t size = 0;
	sscanf(argv[1], "%zu", &size);
	printf("allocating %f MB\n", size / (1024.0f * 1024.0f));

	int* ptr = NULL;

#ifdef MAP
	ptr = mmap(NULL,
		   size * sizeof(int),
		   PROT_READ | PROT_WRITE,
		   MAP_ANONYMOUS | MAP_PRIVATE,
		   -1, 0);
	if (ptr == MAP_FAILED) {
		printf("couldn't map memory\n");
		return 1;
	}
#else
	ptr = malloc(size * sizeof(int));	
	if (ptr == NULL) {
		printf("couldn't allocate memory\n");
		return 1;
	}
#endif

	for (int i = 0; i < size; i++) {
		ptr[i] = rand() % 100;
	}

	long sum = 0;
	for (int i = 0; i < size; i++) {
		sum += ptr[i];
	}

	printf("%ld\n", sum);

#ifdef MAP
	munmap(ptr, size * sizeof(int));
#else
	free(ptr);
#endif

	return 0;

}
