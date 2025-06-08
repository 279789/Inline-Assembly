#include <stdio.h>

int main() {
	int a = 42;

	asm volatile (
		"movl $100, %%eax\n"
		"addl %%eax, %[val]\n"
		: [val] "+r" (a)
		:
		: "%eax"

	);

	printf("Final value: %d\n", a);
	return 0;
}
