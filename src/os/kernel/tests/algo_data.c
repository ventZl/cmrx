#include "algo_data.h"
#include <kernel/algo.h>

// To force hashing function emission
uint32_t os_hash_key(uint32_t key);

Entry_t entries[ENTRIES_MAX];

// Template for reinitializing array before every test
const Entry_t default_content[]= {
    { .id = 42, .data = 0x0BADF00D },
    { .id = 69, .data = 0x12345678 }
};
// Amount of entries in the default content
const unsigned default_content_size = sizeof(default_content) / sizeof(default_content[0]);

unsigned entries_size;
