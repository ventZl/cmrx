#pragma once

#include <kernel/syscall.h>

struct Syscall_Entry_t * os_syscalls_start(void);
struct Syscall_Entry_t * os_syscalls_end(void);

#define REGISTER_SYSCALLS(...)