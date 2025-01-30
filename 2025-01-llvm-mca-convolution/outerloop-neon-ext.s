        add     x4, x1, x2
        ldp q0, q1, [x4]
        ext v11.16b, v0.16b, v1.16b, 4
        ext v12.16b, v0.16b, v1.16b, 8
        ext v13.16b, v0.16b, v1.16b, 12
        fmul v4.4s, v0.4s, v7.s[0]
        fmla v4.4s, v11.4s, v7.s[1]
        fmla v4.4s, v12.4s, v7.s[2]
        fmla v4.4s, v13.4s, v7.s[3]
        fmla v4.4s, v1.4s, v6.s[0]
        str     q4, [x0, x2]
        add     x2, x2, 16
        cmp     x2, x3

