#include "setjmp_arm.h"
#pragma thumb off

asm int setjmp_arm(register jmp_buf_arm env)
{
	stmia a1,{v1-v8,sp,lr}
	mov a1,#0 // return 0
	bx lr
}

asm void longjmp_arm(register jmp_buf_arm env, register int value)
{
	ldmia a1,{v1-v8,sp,lr}
	movs a1,a2 // return value as setjmp() result code
	moveq a1,#1 // return 1 instead of 0
	bx lr
}