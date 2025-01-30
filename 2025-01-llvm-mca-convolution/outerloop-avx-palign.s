vmovups (%r12, %rax, 4), %ymm0
vmovups 16(%r12, %rax, 4), %ymm4
vpalignr $4, %ymm0, %ymm4, %ymm1
vpalignr $8, %ymm0, %ymm4, %ymm2
vpalignr $12, %ymm0, %ymm4, %ymm3
vmulps %ymm0, %ymm6, %ymm5
vmulps %ymm1, %ymm7, %ymm1
vmulps %ymm2, %ymm8, %ymm2
vmulps %ymm3, %ymm9, %ymm3
vmulps %ymm4, %ymm10, %ymm4
vaddps %ymm5, %ymm1, %ymm5
vaddps %ymm5, %ymm2, %ymm5
vaddps %ymm5, %ymm3, %ymm5
vaddps %ymm5, %ymm4, %ymm5
vmovups %ymm5, (%rbx,%rax,4)
addq    $8, %rax
cmpq    %rcx, %rax
