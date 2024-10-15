#include <cmrx/cmrx.h>
#include <cmrx/sys/syscalls.h>

__SYSCALL long get_cpu_freq(void)
{
    __SVC(SYSCALL_CPUFREQ_GET);
}


