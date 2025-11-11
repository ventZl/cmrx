#pragma once

extern struct Syscall_Entry_t __syscall_start;
extern struct Syscall_Entry_t __syscall_end;

struct Syscall_Entry_t * os_syscalls_start(void)
{
    return &__syscall_start;
}

struct Syscall_Entry_t * os_syscalls_end(void)
{
    return &__syscall_end;
}


#define REGISTER_SYSCALLS(...) \
static SYSCALL_DEFINITION struct Syscall_Entry_t syscalls_ ## __COUNTER__[] = { __VA_ARGS__ }

