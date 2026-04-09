#pragma once

#include <cmrx/defines.h>

#define __SYSCALL

#define CMRX_SYSCALL_GET_ARG_COUNT_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...)	N
#ifdef __clang__
#   define CMRX_SYSCALL_GET_ARG_COUNT(...)			CMRX_SYSCALL_GET_ARG_COUNT_HELPER(10, ## __VA_ARGS__ , 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#else
#   define CMRX_SYSCALL_GET_ARG_COUNT(...)			CMRX_SYSCALL_GET_ARG_COUNT_HELPER(__VA_ARGS__ __VA_OPT__(,) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

int system_call_entrypoint(unsigned long arg0,
                            unsigned long arg1,
                            unsigned long arg2,
                            unsigned long arg3,
                           unsigned long arg4,
                           unsigned long arg5,
                           unsigned char syscall_id);

#define CMRX_SYSCALL_PASTER(argcount)	            CMRX_SYSCALL_CALL_ ## argcount
#define CMRX_SYSCALL_EVALUATOR(argcount)	        CMRX_SYSCALL_PASTER(argcount)

#define CMRX_SYSCALL_CALL_6(no, arg0, arg1, arg2, arg3, arg4, arg5) \
system_call_entrypoint((unsigned long) arg0, \
(unsigned long) arg1, \
(unsigned long) arg2, \
(unsigned long) arg3, \
(unsigned long) arg4, \
(unsigned long) arg5, \
no)
#define CMRX_SYSCALL_CALL_5(no, arg0, arg1, arg2, arg3, arg4) \
system_call_entrypoint((unsigned long) arg0, \
(unsigned long) arg1, \
(unsigned long) arg2, \
(unsigned long) arg3, \
(unsigned long) arg4, \
0, no)
#define CMRX_SYSCALL_CALL_4(no, arg0, arg1, arg2, arg3) \
    system_call_entrypoint((unsigned long) arg0, \
                        (unsigned long) arg1, \
                        (unsigned long) arg2, \
                        (unsigned long) arg3, \
                        0, 0, no)
#define CMRX_SYSCALL_CALL_3(no, arg0, arg1, arg2) system_call_entrypoint(\
    (unsigned long) arg0, \
    (unsigned long) arg1, \
    (unsigned long) arg2, \
    0, 0, 0, no)
#define CMRX_SYSCALL_CALL_2(no, arg0, arg1) system_call_entrypoint(\
    (unsigned long) arg0, \
    (unsigned long) arg1, \
    0, 0, 0, 0, no)
#define CMRX_SYSCALL_CALL_1(no, arg0) system_call_entrypoint(\
    (unsigned long) arg0, \
    0, 0, 0, 0, 0, no)
#define CMRX_SYSCALL_CALL_0(no) system_call_entrypoint(0, 0, 0, 0, 0, 0, no)

#define __SVC(no, ...)   \
    return CMRX_SYSCALL_EVALUATOR(CMRX_SYSCALL_GET_ARG_COUNT(__VA_ARGS__))(\
    no, ##__VA_ARGS__)
