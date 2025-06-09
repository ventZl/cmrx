#pragma once

#include <kernel/syscall.h>

struct Syscall_Entry_t * os_syscalls_start(void);
struct Syscall_Entry_t * os_syscalls_end(void);

extern void cmrx_posix_register_syscalls(struct Syscall_Entry_t * syscalls);

#define REGISTER_SYSCALLS(...) \
static struct Syscall_Entry_t syscalls[] = { __VA_ARGS__, { 0, 0} };\
__attribute__((constructor)) void register_syscalls_ ## __COUNTER__(void)\
{\
    cmrx_posix_register_syscalls(syscalls);\
}
