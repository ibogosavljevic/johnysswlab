        add     x4, x1, x2
        ldr q0, [x4]
        ldr q1, [x4, #4]
        ldr q2, [x4, #8]
        ldr q3, [x4, #12]
        ldr q4, [x4, #16]
        fmul v15.4s, v0.4s, v7.s[0]
        fmla v15.4s, v1.4s, v7.s[1]
        fmla v15.4s, v2.4s, v7.s[2]
        fmla v15.4s, v3.4s, v7.s[3]
        fmla v15.4s, v4.4s, v6.s[0]
        str     q15, [x0, x2]
        add     x2, x2, 16
        cmp     x2, x3

