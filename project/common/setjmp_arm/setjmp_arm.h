#ifdef __cplusplus
extern "C" {
#endif

#pragma options align=native
typedef long *jmp_buf_arm[10]; // V1-V8, SP, LR (see setjmp.c)
#pragma options align=reset

int setjmp_arm(register jmp_buf_arm env);
void longjmp_arm(register jmp_buf_arm env, register int value);

#ifdef __cplusplus
}
#endif