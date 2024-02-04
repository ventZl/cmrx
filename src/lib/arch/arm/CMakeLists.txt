add_library(stdlib_arch OBJECT mutex.c)
target_link_libraries(stdlib_arch PUBLIC cmsis_core_lib)
target_include_directories(stdlib_arch PUBLIC ${HAL_PATH})
