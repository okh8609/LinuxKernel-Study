.section .data
arr_len:
    .quad 7

arr:
    .quad 22
    .quad 0
    .quad 72
    .quad 81
    .quad 23
    .quad 5
    .quad 99
    
fmt_str:
    .string "big data: %d\n"



.section .text
.globl main
main: 
    STP X29, X30, [SP, -16]!

    LDR X0, arr_len
    LDR X1, =arr

    ADD X2, X1, 48

    MOV X3, XZR

8: 
    LDR X4, [X1], 8
    CMP X4, X3
    CSEL X3, X4, X3, HI
    CMP X1, X2
    B.LS 8b

    LDR X0, =fmt_str
    MOV X1, X3
    
    BL printf

    LDP X29, X30, [SP], 16
    RET

