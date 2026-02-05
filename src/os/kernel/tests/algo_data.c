#include "algo_data.h"



Entry_t entries[ENTRIES_MAX];

// Template for reinitializing array before every test
const Entry_t default_content[]= {
    { .id = 42, .data = 0x0BADF00D },
    { .id = 69, .data = 0x12345678 }
};
// Amount of entries in the default content
const unsigned default_content_size = sizeof(default_content) / sizeof(default_content[0]);

unsigned entries_size;
