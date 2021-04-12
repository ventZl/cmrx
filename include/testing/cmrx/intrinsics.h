#pragma once

extern long unsigned * LR;
extern long unsigned * PSP;

long unsigned * __get_LR() { return LR; }

long unsigned * __get_PSP() { return PSP; }
void __set_PSP(long unsigned * psp) { PSP = psp; }
void __set_CONTROL(unsigned control) { return; }

void __DSB() { return; }

void __ISB() { return; }

void __tail_call(void (*return_to)(int), int (*entrypoint)(void *), void * data) { return; }
