#include <ctest.h>
#include <kernel/algo.h>
#include "algo_data.h"

CTEST_DATA(algo_insert_sort) {
};

CTEST_SETUP(algo_insert_sort) {
    for (unsigned q = 0; q < default_content_size; ++q)
    {
        entries[q] = default_content[q];
    }

    entries_size = default_content_size;
}

/* Check if array is resized if it is empty with key not matchin anything in the memory */
CTEST2(algo_insert_sort, insert_empty_unique_key) {
    // Make the array appear empty
    entries_size = 0;

    unsigned offs = INSERT_SORT(entries, id, 3, entries_size, ENTRIES_MAX);
    ASSERT_EQUAL(offs, 0);
    ASSERT_EQUAL(entries_size, 1);
}


/* Check if array is resized if it is empty but stale array data has matching key */
CTEST2(algo_insert_sort, insert_empty_matching_key) {
    // Make the array appear empty
    entries_size = 0;

    unsigned offs = INSERT_SORT(entries, id, 42, entries_size, ENTRIES_MAX);
    ASSERT_EQUAL(offs, 0);
    ASSERT_EQUAL(entries_size, 1);
}

/* Check for overwriting entry in array holding one item */
CTEST2(algo_insert_sort, insert_one_matching_key) {
    // Make the array appear empty
    entries_size = 1;

    unsigned offs = INSERT_SORT(entries, id, 42, entries_size, ENTRIES_MAX);
    ASSERT_EQUAL(offs, 0);
    ASSERT_EQUAL(entries_size, 1);
}

/* Check for insertion of entries with non-matching keys */
CTEST2(algo_insert_sort, insert_one_missing_keys) {
    // Make the array appear empty
    entries_size = 1;

    unsigned offs = INSERT_SORT(entries, id, 43, entries_size, ENTRIES_MAX);
    ASSERT_EQUAL(offs, 1);
    ASSERT_EQUAL(entries_size, 2);

    // To keep array sorted, so the next part of the test yields predictable result
    entries[offs].id = 43;

    offs = INSERT_SORT(entries, id, 41, entries_size, ENTRIES_MAX);
    ASSERT_EQUAL(offs, 0);
    ASSERT_EQUAL(entries_size, 3);
}
