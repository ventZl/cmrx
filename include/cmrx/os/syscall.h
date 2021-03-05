#pragma once

typedef int (* Syscall_Handler_t)(int, int, int, int);

struct Syscall_Entry_t {
	uint8_t id;
	Syscall_Handler_t handler;
};


