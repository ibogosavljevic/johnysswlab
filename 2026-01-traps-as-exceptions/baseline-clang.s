.intel_syntax noprefix
.text
.globl loop
.type loop, @function

loop:
    vsqrtpd ymm0, YMMWORD PTR [rbx + rax*8 - 0x80]
    vsqrtpd ymm1, YMMWORD PTR [rbx + rax*8 - 0x60]
    vsqrtpd ymm2, YMMWORD PTR [rbx + rax*8 - 0x40]

    vmovupd YMMWORD PTR [r14 + rax*8 - 0x80], ymm0
    vmovupd YMMWORD PTR [r14 + rax*8 - 0x60], ymm1
    vmovupd YMMWORD PTR [r14 + rax*8 - 0x40], ymm2

    vsqrtpd ymm0, YMMWORD PTR [rbx + rax*8 - 0x20]
    vsqrtpd ymm1, YMMWORD PTR [rbx + rax*8]

    vmovupd YMMWORD PTR [r14 + rax*8 - 0x20], ymm0
    vmovupd YMMWORD PTR [r14 + rax*8],        ymm1

    add rax, 0x14
    cmp rax, 0xfa010
    jne loop
