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


__SYSCALL uint32_t shutdown(void)
{
    __SVC(SYSCALL_SHUTDOWN);
}

uint32_t diff_microtime(uint32_t older_timestamp, uint32_t newer_timestamp)
{
    if (older_timestamp <= newer_timestamp)
    {
        return newer_timestamp - older_timestamp;
    }
    else
    {
        return ~0 - (older_timestamp - newer_timestamp);
    }
}
