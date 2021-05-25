#ifdef __ARMEB__
#define xh r0
#define xl r1
#define yh r2
#define yl r3
#else
#define xl r0
#define xh r1
#define yl r2
#define yh r3
#endif

.global __muldi3
__muldi3:

.global __aeabi_lmul
__aeabi_lmul:

        mul     xh, yl, xh
        mla     xh, xl, yh, xh
        mov     ip, xl, lsr #16
        mov     yh, yl, lsr #16
        bic     xl, xl, ip, lsl #16
        bic     yl, yl, yh, lsl #16
        mla     xh, yh, ip, xh
        mul     yh, xl, yh
        mul     xl, yl, xl
        mul     ip, yl, ip
        adds    xl, xl, yh, lsl #16
        adc     xh, xh, yh, lsr #16
        adds    xl, xl, ip, lsl #16
        adc     xh, xh, ip, lsr #16
        mov     pc, lr


dividend    .req    r0
divisor     .req    r1
result      .req    r2
curbit      .req    r3
.globl   __udivsi3
    .type   __udivsi3 ,function
    .globl  __aeabi_uidiv
    .type   __aeabi_uidiv ,function
    .align  0
 __udivsi3:
 __aeabi_uidiv:
    cmp divisor, #0
    beq Ldiv0_uidiv
    mov curbit, #1
    mov result, #0
    cmp dividend, divisor
    bcc Lgot_result
Loop1:
    @ Unless the divisor is very big, shift it up in multiples of
    @ four bits, since this is the amount of unwinding in the main
    @ division loop.  Continue shifting until the divisor is
    @ larger than the dividend.
    cmp divisor, #0x10000000
    cmpcc   divisor, dividend
    movcc   divisor, divisor, lsl #4
    movcc   curbit, curbit, lsl #4
    bcc Loop1
Lbignum:
    @ For very big divisors, we must shift it a bit at a time, or
    @ we will be in danger of overflowing.
    cmp divisor, #0x80000000
    cmpcc   divisor, dividend
    movcc   divisor, divisor, lsl #1
    movcc   curbit, curbit, lsl #1
    bcc Lbignum
Loop3:
    @ Test for possible subtractions, and note which bits
    @ are done in the result.  On the final pass, this may subtract
    @ too much from the dividend, but the result will be ok, since the
    @ "bit" will have been shifted out at the bottom.
    cmp dividend, divisor
    subcs   dividend, dividend, divisor
    orrcs   result, result, curbit
    cmp dividend, divisor, lsr #1
    subcs   dividend, dividend, divisor, lsr #1
    orrcs   result, result, curbit, lsr #1
    cmp dividend, divisor, lsr #2
    subcs   dividend, dividend, divisor, lsr #2
    orrcs   result, result, curbit, lsr #2
    cmp dividend, divisor, lsr #3
    subcs   dividend, dividend, divisor, lsr #3
    orrcs   result, result, curbit, lsr #3
    cmp dividend, #0            @ Early termination?
    movnes  curbit, curbit, lsr #4      @ No, any more bits to do?
    movne   divisor, divisor, lsr #4
    bne Loop3
Lgot_result:
    mov r0, result
    mov pc, lr
Ldiv0_uidiv:
    str lr, [sp, #-4]!
    #bl  __div0       (PLT)
    mov r0, #0          @ about as wrong as it could be
    ldmia   sp!, {pc}
    .size  __udivsi3       , . -  __udivsi3

.globl __aeabi_uidivmod
__aeabi_uidivmod:

    stmfd   sp!, {r0, r1, ip, lr}
    bl  __aeabi_uidiv
    ldmfd   sp!, {r1, r2, ip, lr}
    mul r3, r0, r2
    sub r1, r1, r3
    mov pc, lr

.globl __aeabi_idivmod
__aeabi_idivmod:

    stmfd   sp!, {r0, r1, ip, lr}
    bl  __aeabi_idiv
    ldmfd   sp!, {r1, r2, ip, lr}
    mul r3, r0, r2
    sub r1, r1, r3
    mov pc, lr

.macro ARM_DIV_BODY dividend, divisor, result, curbit

#if __LINUX_ARM_ARCH__ >= 5

    clz \curbit, \divisor
    clz \result, \dividend
    sub \result, \curbit, \result
    mov \curbit, #1
    mov \divisor, \divisor, lsl \result
    mov \curbit, \curbit, lsl \result
    mov \result, #0

#else

    @ Initially shift the divisor left 3 bits if possible,
    @ set curbit accordingly.  This allows for curbit to be located
    @ at the left end of each 4 bit nibbles in the division loop
    @ to save one loop in most cases.
    tst \divisor, #0xe0000000
    moveq   \divisor, \divisor, lsl #3
    moveq   \curbit, #8
    movne   \curbit, #1

    @ Unless the divisor is very big, shift it up in multiples of
    @ four bits, since this is the amount of unwinding in the main
    @ division loop.  Continue shifting until the divisor is
    @ larger than the dividend.
1:  cmp \divisor, #0x10000000
    cmplo   \divisor, \dividend
    movlo   \divisor, \divisor, lsl #4
    movlo   \curbit, \curbit, lsl #4
    blo 1b

    @ For very big divisors, we must shift it a bit at a time, or
    @ we will be in danger of overflowing.
1:  cmp \divisor, #0x80000000
    cmplo   \divisor, \dividend
    movlo   \divisor, \divisor, lsl #1
    movlo   \curbit, \curbit, lsl #1
    blo 1b

    mov \result, #0

#endif

    @ Division loop
1:  cmp \dividend, \divisor
    subhs   \dividend, \dividend, \divisor
    orrhs   \result,   \result,   \curbit
    cmp \dividend, \divisor,  lsr #1
    subhs   \dividend, \dividend, \divisor, lsr #1
    orrhs   \result,   \result,   \curbit,  lsr #1
    cmp \dividend, \divisor,  lsr #2
    subhs   \dividend, \dividend, \divisor, lsr #2
    orrhs   \result,   \result,   \curbit,  lsr #2
    cmp \dividend, \divisor,  lsr #3
    subhs   \dividend, \dividend, \divisor, lsr #3
    orrhs   \result,   \result,   \curbit,  lsr #3
    cmp \dividend, #0           @ Early termination?
    movnes  \curbit,   \curbit,  lsr #4 @ No, any more bits to do?
    movne   \divisor,  \divisor, lsr #4
    bne 1b

.endm

.macro ARM_DIV2_ORDER divisor, order

#if __LINUX_ARM_ARCH__ >= 5

    clz \order, \divisor
    rsb \order, \order, #31

#else

    cmp \divisor, #(1 << 16)
    movhs   \divisor, \divisor, lsr #16
    movhs   \order, #16
    movlo   \order, #0

    cmp \divisor, #(1 << 8)
    movhs   \divisor, \divisor, lsr #8
    addhs   \order, \order, #8

    cmp \divisor, #(1 << 4)
    movhs   \divisor, \divisor, lsr #4
    addhs   \order, \order, #4

    cmp \divisor, #(1 << 2)
    addhi   \order, \order, #3
    addls   \order, \order, \divisor, lsr #1

#endif

.endm

    .align  5
.globl __divsi3
.globl __aeabi_idiv
__divsi3:
__aeabi_idiv:
    cmp r1, #0
    eor ip, r0, r1          @ save the sign of the result.
    beq Ldiv0
    rsbmi   r1, r1, #0          @ loops below use unsigned.
    subs    r2, r1, #1          @ division by 1 or -1 ?
    beq 10f
    movs    r3, r0
    rsbmi   r3, r0, #0          @ positive dividend value
    cmp r3, r1
    bls 11f
    tst r1, r2              @ divisor is power of 2 ?
    beq 12f

    ARM_DIV_BODY r3, r1, r0, r2

    cmp ip, #0
    rsbmi   r0, r0, #0
    mov pc, lr

10: teq ip, r0              @ same sign ?
    rsbmi   r0, r0, #0
    mov pc, lr

11: movlo   r0, #0
    moveq   r0, ip, asr #31
    orreq   r0, r0, #1
    mov pc, lr

12: ARM_DIV2_ORDER r1, r2

    cmp ip, #0
    mov r0, r3, lsr r2
    rsbmi   r0, r0, #0
    mov pc, lr

Ldiv0:

    str lr, [sp, #-4]!
    #bl __div0
    mov r0, #0          @ About as wrong as it could be.
    ldr pc, [sp], #4


.global __aeabi_uldivmod
    .type   __aeabi_uldivmod, function
    .align  0
A_0 .req    r0
A_1 .req    r1
B_0 .req    r2
B_1 .req    r3
C_0 .req    r4
C_1 .req    r5
D_0 .req    r6
D_1 .req    r7
Q_0 .req    r0
Q_1 .req    r1
R_0 .req    r2
R_1 .req    r3
__aeabi_uldivmod:
    stmfd   sp!, {r4, r5, r6, r7, lr}
    @ Test if B == 0
    orrs    ip, B_0, B_1        @ Z set -> B == 0
    beq L_div_by_0
    @ Test if B is power of 2: (B & (B - 1)) == 0
    subs    C_0, B_0, #1
    sbc C_1, B_1, #0
    tst C_0, B_0
    tsteq   B_1, C_1
    beq L_pow2
    @ Test if A_1 == B_1 == 0
    orrs    ip, A_1, B_1
    beq L_div_32_32
L_div_64_64:
    mov C_0, #1
    mov C_1, #0
    @ D_0 = clz A
    teq A_1, #0
    clz D_0, A_1
    clzeq   ip, A_0
    addeq   D_0, D_0, ip
    @ D_1 = clz B
    teq B_1, #0
    clz D_1, B_1
    clzeq   ip, B_0
    addeq   D_1, D_1, ip
    @ if clz B - clz A > 0
    subs    D_0, D_1, D_0
    bls L_done_shift
    @ B <<= (clz B - clz A)
    subs    D_1, D_0, #32
    rsb ip, D_0, #32
    movmi   B_1, B_1, lsl D_0
    orrmi   B_1, B_1, B_0, lsr ip
    movpl   B_1, B_0, lsl D_1
    mov B_0, B_0, lsl D_0
    @ C = 1 << (clz B - clz A)
    movmi   C_1, C_1, lsl D_0
    orrmi   C_1, C_1, C_0, lsr ip
    movpl   C_1, C_0, lsl D_1
    mov C_0, C_0, lsl D_0
L_done_shift:
    mov D_0, #0
    mov D_1, #0
    @ C: current bit; D: result
L_subtract:
    @ if A >= B
    cmp A_1, B_1
    cmpeq   A_0, B_0
    bcc L_update
    @ A -= B
    subs    A_0, A_0, B_0
    sbc A_1, A_1, B_1
    @ D |= C
    orr D_0, D_0, C_0
    orr D_1, D_1, C_1
L_update:
    @ if A == 0: break
    orrs    ip, A_1, A_0
    beq L_exit
    @ C >>= 1
    movs    C_1, C_1, lsr #1
    movs    C_0, C_0, rrx
    @ if C == 0: break
    orrs    ip, C_1, C_0
    beq L_exit
    @ B >>= 1
    movs    B_1, B_1, lsr #1
    mov B_0, B_0, rrx
    b   L_subtract
L_exit:
    @ Note: A, B & Q, R are aliases
    mov R_0, A_0
    mov R_1, A_1
    mov Q_0, D_0
    mov Q_1, D_1
    ldmfd   sp!, {r4, r5, r6, r7, pc}
L_div_32_32:
    @ Note: A_0 &   r0 are aliases
    @   Q_1 r1
    mov r1, B_0
    bl  __aeabi_uidivmod
    mov R_0, r1
    mov R_1, #0
    mov Q_1, #0
    ldmfd   sp!, {r4, r5, r6, r7, pc}
L_pow2:
    @ Note: A, B and Q, R are aliases
    @ R = A & (B - 1)
    and C_0, A_0, C_0
    and C_1, A_1, C_1
    @ Q = A >> log2(B)
    @ Note: B must not be 0 here!
    clz D_0, B_0
    add D_1, D_0, #1
    rsbs    D_0, D_0, #31
    bpl L_1
    clz D_0, B_1
    rsb D_0, D_0, #31
    mov A_0, A_1, lsr D_0
    add D_0, D_0, #32
L_1:
    movpl   A_0, A_0, lsr D_0
    orrpl   A_0, A_0, A_1, lsl D_1
    mov A_1, A_1, lsr D_0
    @ Mov back C to R
    mov R_0, C_0
    mov R_1, C_1
    ldmfd   sp!, {r4, r5, r6, r7, pc}
L_div_by_0:
    #bl __div0
    @ As wrong as it could be
    mov Q_0, #0
    mov Q_1, #0
    mov R_0, #0
    mov R_1, #0
    ldmfd   sp!, {r4, r5, r6, r7, pc}

