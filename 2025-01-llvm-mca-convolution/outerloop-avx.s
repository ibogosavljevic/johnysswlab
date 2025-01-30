leaq    0(%r13,%rax), %rcx
vmovaps %ymm10, %ymm0
vmulps (%rcx), %ymm5, %ymm0
vmulps 4(%rcx), %ymm6, %ymm1
vmulps 8(%rcx), %ymm7, %ymm2
vmulps 12(%rcx), %ymm8, %ymm3
vmulps 16(%rcx), %ymm9, %ymm4
vaddps %ymm0, %ymm1, %ymm0
vaddps %ymm0, %ymm2, %ymm0
vaddps %ymm0, %ymm3, %ymm0
vaddps %ymm0, %ymm4, %ymm0
vmovups %ymm0, (%r12,%rax)
addq    $32, %rax
cmpq    %rdx, %rax
