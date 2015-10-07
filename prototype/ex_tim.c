#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>

#define MAX 100 
int main()
{
	printf("hello\n"); 
	int *a;
	int * test;
	unsigned int s = 0;
	unsigned int s2 = 0;

	a = (malloc(MAX *sizeof(double)));
	printf("ok?\n");
	test = (int*)mmap(NULL,MAX,PROT_READ|PROT_WRITE, MAP_SHARED,-1,0);//probleme mon capitain
	if(test == MAP_FAILED)
		errx(1 , "we have a mmap problem\n"); // ?
	unsigned int i = 0;
	for (i = 0; i < MAX; i++) {
		a[i] = 1;
		test[i] = 1;
	}
	for (i = 0; i < MAX; i++) {
		s += a[i];
		s2 += test[i];
	}
	printf("s = %u et s2 = %u \n",s,s2);

	free(a);
	munmap(test, MAX);
	


return 0;

}