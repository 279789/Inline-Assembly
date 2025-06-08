#include<stdio.h>

int main() {
	int a = 5, b = 7, result;

	asm volatile (
		"addl %[output_b], %[output_a]"
		: [output_res] "=r" (result),  [output_a] "=r" (a), [output_b] "=r" (b)
		//: [input_a] "r" (a), [input_b] "r" (b)
		);

	printf("Result: %d\n", result);
	return 0;

}
