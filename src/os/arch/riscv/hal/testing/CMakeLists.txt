target_sources(os PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv_hal_fake.c
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv_context_switch_fake.c
)

target_sources(test_kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/riscv_context_switch.c
)
