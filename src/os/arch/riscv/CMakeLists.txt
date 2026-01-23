set(cmrx_riscv_SRCS
    hal/riscv_hal_backend.c
    context_switch_core.c
    context_switch_request.c
)

target_sources(os PRIVATE ${cmrx_riscv_SRCS})

