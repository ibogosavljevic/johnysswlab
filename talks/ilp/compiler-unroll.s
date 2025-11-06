.intel_syntax

loop_begin:

vmovdqu ymm0, ymmword ptr [rdx + r9]             # ymm0 = load(a{rdx} + i{r9})
vmovdqu ymm1, ymmword ptr [rdx + r9 + 32]        # ymm0 = load(a{rdx} + i{r9} + 32)
vmovdqu ymm2, ymmword ptr [rdx + r9 + 64]        # ymm0 = load(a{rdx} + i{r9} + 64)
vmovdqu ymm3, ymmword ptr [rdx + r9 + 96]        # ymm0 = load(a{rdx} + i{r9} + 96)
vpaddd  ymm0, ymm0, ymmword ptr [rsi + r9]       # ymm0 = ymm0 + load(b{rsi} + i{r9})
vpaddd  ymm1, ymm1, ymmword ptr [rsi + r9 + 32]  # ymm1 = ymm1 + load(b{rsi} + i{r9})
vpaddd  ymm2, ymm2, ymmword ptr [rsi + r9 + 64]  # ymm2 = ymm2 + load(b{rsi} + i{r9})
vpaddd  ymm3, ymm3, ymmword ptr [rsi + r9 + 96]  # ymm3 = ymm3 + load(b{rsi} + i{r9})
vmovdqu ymmword ptr [rdi + r9], ymm0             # store(c{rdi} + i{r9}, ymm0)
vmovdqu ymmword ptr [rdi + r9 + 32], ymm1        # store(c{rdi} + i{r9} + 32, ymm1)
vmovdqu ymmword ptr [rdi + r9 + 64], ymm2        # store(c{rdi} + i{r9} + 64, ymm2)
vmovdqu ymmword ptr [rdi + r9 + 96], ymm3        # store(c{rdi} + i{r9} + 96, ymm3)
sub     r9, -128                                 # i{r9} = i{r9} + 128
cmp     r8, r9                                   # compare i{r9} and n{r8}
jne    loop_begin                                # jump if not equal to loop begin
