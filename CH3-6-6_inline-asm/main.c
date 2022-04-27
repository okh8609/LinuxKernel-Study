#include <stdio.h>

static int max2(int a, int b)
{
	int val;

	// asm(code
	//    : output operand list
	//    : input operand list
	//    : clobber list->告訴 compiler 有哪些 register 被這段 assembly code 修改了。
	//    );

	asm volatile(
		"cmp %1, %2\n"
		"csel %0, %1, %2, hi\n"
		: "+r"(val)
		: "r"(a), "r"(b)
		: "memory"); //告訴 compiler memory 有可能會被修改。

	return val;
}

int main()
{
	int val;

	val = max2(5, 6);
	printf("big data: %d\n", val);

	val = max2(6, 4);
	printf("big data: %d\n", val);
}
