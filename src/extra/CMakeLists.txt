set(aux_systick_SRCS systick.c)
add_library(aux_systick STATIC ${aux_systick_SRCS})
target_link_libraries(aux_systick cmsis_core_lib cmsis_core)

set(aux_ipi_SRCS cm_ipi.c)
add_library(aux_ipi STATIC ${aux_ipi_SRCS})
target_link_libraries(aux_ipi cmsis_core_lib cmsis_core)
