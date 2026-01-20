set(cmrx_riscv_SRCS
    hal/riscv_hal_backend.c
)

target_sources(os PRIVATE ${cmrx_riscv_SRCS})

