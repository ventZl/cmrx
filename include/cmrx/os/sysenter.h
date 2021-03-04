#pragma once

#include <stdint.h>

#define __SYSCALL		__attribute__((naked)) __attribute__((noinline))

#define ___SVC(no)\
	asm volatile(\
			"SVC " #no "\n\t"\
			"BX LR"\
			)

#define __SVC(no) ___SVC(no)

