#include "syscall.h"

int x[10000];

int main() {
	int n;
	for (n=9; n>5; n--) {
		PrintInt(n);
	}
	return 0;
	//Halt();
}
