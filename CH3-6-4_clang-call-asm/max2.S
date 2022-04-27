.section .text
.globl max2
max2:
    CMP X1, X0
    CSEL X9, X1, X0, HI
    RET
