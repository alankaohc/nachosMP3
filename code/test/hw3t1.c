#include "syscall.h"
// hw3t1.c
int
main()
{
	int n, i;
	for (n = 1; n < 10; ++n) {
		PrintInt(1);
		for (i=0; i<200; ++i);
	}
	Exit(1);
}
