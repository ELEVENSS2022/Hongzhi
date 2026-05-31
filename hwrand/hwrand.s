.intel_syntax noprefix
.text
.globl hwrand

hwrand:
    mov ecx,10
1:
    rdrand rax
    jc 2f
    dec ecx
    jnz 1b
    xor rax,rax
2:
    ret
