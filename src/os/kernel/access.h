#pragma once

#include <stdbool.h>
#include <stdint.h>

enum AccessType {
    ACCESS_READ_WRITE,
    ACCESS_EXECUTE
};

bool os_thread_check_access(uint8_t thread_id, void * address, int access_type);
