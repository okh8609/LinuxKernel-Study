.section .data
.align 3
fmt_str:
    .string "biggest: %d\n"

.section .text
.globl main
main:
	stp     x29, x30, [sp, -16]!


    mov x0, 76
    mov x1, 54
    mov x2, 32
    bl max3

    mov x1, x0
    ldr x0, =fmt_str
    bl printf

	ldp  x29, x30, [sp], 16
	ret