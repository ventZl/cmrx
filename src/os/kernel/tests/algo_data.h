#pragma once

typedef struct {
    unsigned id;
    unsigned data;
} Entry_t;

#define ENTRIES_MAX 16

extern Entry_t entries[ENTRIES_MAX];

extern const Entry_t default_content[];
extern const unsigned default_content_size;
extern unsigned entries_size;
