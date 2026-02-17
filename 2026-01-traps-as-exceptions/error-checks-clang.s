.intel_syntax noprefix

loop2:
    vmovupd ymm0, YMMWORD PTR [rbx + rax*8 + 0x20]
    vsqrtpd ymm1, ymm0
    vmovupd YMMWORD PTR [rcx + rax*8 + 0x20], ymm1
    vcmplepd ymm0, ymm3, ymm0
    vpsubq ymm0, ymm2, ymm0

    vmovupd ymm1, YMMWORD PTR [rbx + rax*8 + 0x40]
    vsqrtpd ymm2, ymm1
    vmovupd YMMWORD PTR [rcx + rax*8 + 0x40], ymm2
    vcmplepd ymm1, ymm3, ymm1
    vpsubq ymm2, ymm0, ymm1

    add rax, 0x8
    cmp rax, 0xf9ffc
    jb loop2
