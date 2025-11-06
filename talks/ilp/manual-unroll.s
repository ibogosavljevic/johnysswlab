.intel_syntax

loop_begin:

vmovdqu ymm0, ymmword ptr [rdx + r9]          # ymm0 = load(a{rdx} + i{r9})
vpaddd  ymm0, ymm0, ymmword ptr [rsi + r9]    # ymm0 = ymm0 + load(b{rdx} + i{r9})
vmovdqu ymmword ptr [rdi + r9], ymm0          # store(c{rdi} + i{r9})
add     r9, 32                                # i{r9} = i{r9} + 32

vmovdqu ymm0, ymmword ptr [rdx + r9]          # ymm0 = load(a{rdx} + i{r9})
vpaddd  ymm0, ymm0, ymmword ptr [rsi + r9]    # ymm0 = ymm0 + load(b{rdx} + i{r9})
vmovdqu ymmword ptr [rdi + r9], ymm0          # store(c{rdi} + i{r9})
add     r9, 32                                # i{r9} = i{r9} + 32

vmovdqu ymm0, ymmword ptr [rdx + r9]          # ymm0 = load(a{rdx} + i{r9})
vpaddd  ymm0, ymm0, ymmword ptr [rsi + r9]    # ymm0 = ymm0 + load(b{rdx} + i{r9})
vmovdqu ymmword ptr [rdi + r9], ymm0          # store(c{rdi} + i{r9})
add     r9, 32                                # i{r9} = i{r9} + 32

vmovdqu ymm0, ymmword ptr [rdx + r9]          # ymm0 = load(a{rdx} + i{r9})
vpaddd  ymm0, ymm0, ymmword ptr [rsi + r9]    # ymm0 = ymm0 + load(b{rdx} + i{r9})
vmovdqu ymmword ptr [rdi + r9], ymm0          # store(c{rdi} + i{r9})
add     r9, 32                                # i{r9} = i{r9} + 32

cmp     r8, r9                                # compare i{r9} and n{r8}
jne     loop_begin                            # jump not equal to loop_begin 
