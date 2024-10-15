#include <cmrx/cmrx.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL long get_cpu_freq(void)
{
    __SVC(SYSCALL_CPUFREQ_GET);
}


__SYSCALL uint32_t getmicrotime(void)
{
    __SVC(SYSCALL_GET_MICROTIME);
}
