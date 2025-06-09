#pragma once

struct Syscall_Entry_t * os_syscalls_start(void);
struct Syscall_Entry_t * os_syscalls_end(void);

#define REGISTER_SYSCALLS(...) \
static struct Syscall_Entry_t syscalls[] = { __VA_ARGS__ };

