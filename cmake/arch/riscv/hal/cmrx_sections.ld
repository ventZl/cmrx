/*
 * CMRX linker script fragment for RISC-V.
 *
 * This linker script defines the CMRX static initialization sections
 * and their boundary symbols.
 *
 * The CMRX macros OS_APPLICATION and OS_THREAD_CREATE place data in
 * .applications and .thread_create sections respectively.
 */

SECTIONS
{
    /* Application definitions table */
    .cmrx_applications : {
        PROVIDE(__applications_start = .);
        KEEP(*(.applications))
        KEEP(*(.applications.*))
        PROVIDE(__applications_end = .);
    }

    /* Thread creation table */
    .cmrx_thread_create : {
        PROVIDE(__thread_create_start = .);
        KEEP(*(.thread_create))
        KEEP(*(.thread_create.*))
        PROVIDE(__thread_create_end = .);
    }

    /* Syscall table entries */
    .cmrx_syscall : {
        PROVIDE(__syscall_start = .);
        KEEP(*(.syscall))
        KEEP(*(.syscall.*))
        PROVIDE(__syscall_end = .);
    }
}
