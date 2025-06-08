#include <stdio.h>

int main() {
	int x= 10, y = 20, sum;

	asm volatile (
		"movl %1, %0\n"
		"addl %2, %0\n"
		:"=r" (sum)
		: "r" (x), "r" (y)

	);

	printf("Sum is: %d\n", sum);
	return 0;
}

