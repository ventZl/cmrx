#include <ctest.h>
#include <kernel/algo.h>
#include "algo_data.h"

void hash_insert(uint32_t key)
{
    unsigned offs = HASH_SEARCH(entries, id, key, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    entries[offs].id = key;
}

CTEST_DATA(algo_hash_search) {
};

CTEST_SETUP(algo_hash_search) {
    for (unsigned q = 0; q < ENTRIES_MAX; ++q)
    {
        entries[q].id = HASH_EMPTY;
    }
}

/* Allocate slot for entry in empty table and verify it can be found */
CTEST2(algo_hash_search, find_empty) {
    unsigned offs = HASH_SEARCH(entries, id, 42, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, HASH_EMPTY);

    entries[offs].id = 42;

    unsigned offs2 = HASH_SEARCH(entries, id, 42, ENTRIES_MAX);
    ASSERT_EQUAL(offs, offs2);
    ASSERT_EQUAL(entries[offs].id, 42);
}

/* Insert some entries into table and check that they can be found */
CTEST2(algo_hash_search, find_multiple) {
    // first seed the hash table with some data
    hash_insert(42);
    hash_insert(69);
    hash_insert(12);
    hash_insert(99);
    hash_insert(0x0BADF00DU);

    unsigned offs = HASH_SEARCH(entries, id, 42, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, 42);

    offs = HASH_SEARCH(entries, id, 69, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, 69);

    offs = HASH_SEARCH(entries, id, 12, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, 12);

    offs = HASH_SEARCH(entries, id, 99, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, 99);

    offs = HASH_SEARCH(entries, id, 0x0BADF00DU, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, 0x0BADF00DU);
}

/* Populate table and verify that non-existent keys won't provide false positive lookup result */
CTEST2(algo_hash_search, find_conflicts) {
    // first seed the hash table with some data
    hash_insert(42);
    hash_insert(69);
    hash_insert(12);
    hash_insert(99);
    hash_insert(0x0BADF00DU);

    unsigned offs = HASH_SEARCH(entries, id, 43, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, HASH_EMPTY);

    offs = HASH_SEARCH(entries, id, 0, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, HASH_EMPTY);

    offs = HASH_SEARCH(entries, id, 100, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, HASH_EMPTY);

    offs = HASH_SEARCH(entries, id, 0xBABEDEADU, ENTRIES_MAX);
    ASSERT_LT(offs, ENTRIES_MAX);
    ASSERT_EQUAL(entries[offs].id, HASH_EMPTY);
}
