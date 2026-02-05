#include <ctest.h>
#include <kernel/algo.h>
#include "algo_data.h"

CTEST_DATA(algo_binary_search) {
};

CTEST_SETUP(algo_binary_search) {
    for (unsigned q = 0; q < default_content_size; ++q)
    {
        entries[q] = default_content[q];
    }

    entries_size = default_content_size;
}

/* Search an empty array */
CTEST2(algo_binary_search, search_empty) {
    // Make the array be empty
    entries_size = 0;
    unsigned offs = BINARY_SEARCH(entries, id, 42, entries_size);
    ASSERT_EQUAL(offs, 0);
}

/* Search an array containing one item for exact match */
CTEST2(algo_binary_search, search_one_exact) {
    // Make the array contain one item
    entries_size = 1;
    unsigned offs = BINARY_SEARCH(entries, id, 42, entries_size);
    ASSERT_EQUAL(offs, 0);
}

/* Search an array with one item for non-existent item with value lower than one stored in array */
CTEST2(algo_binary_search, search_one_lower) {
    // Make the array contain one item
    entries_size = 1;
    unsigned offs = BINARY_SEARCH(entries, id, 41, entries_size);
    ASSERT_EQUAL(offs, 0);
}

/* Search an array with one item for non-existent item with value higher than one stored in array */
CTEST2(algo_binary_search, search_one_higher) {
    // Make the array contain one item
    entries_size = 1;
    unsigned offs = BINARY_SEARCH(entries, id, 43, entries_size);
    ASSERT_EQUAL(offs, 1);
}

/* Search an array with one item for non-existent item with value stored outside of the current array capacity */
CTEST2(algo_binary_search, search_beyond_end) {
    // Make the array contain one item
    entries_size = 1;
    unsigned offs = BINARY_SEARCH(entries, id, 70, entries_size);
    ASSERT_EQUAL(offs, 1);
}

/* Search for exact values in array containing two items */
CTEST2(algo_binary_search, search_two_exact) {
    // Make the array contain two items
    entries_size = 2;

    unsigned offs;
    offs = BINARY_SEARCH(entries, id, 42, entries_size);
    ASSERT_EQUAL(offs, 0);

    offs = BINARY_SEARCH(entries, id, 69, entries_size);
    ASSERT_EQUAL(offs, 1);
}

/* Search for nearest values in array containing two items */
CTEST2(algo_binary_search, search_two_nearest) {
    // Make the array contain two items
    entries_size = 2;

    unsigned offs;
    offs = BINARY_SEARCH(entries, id, 41, entries_size);
    ASSERT_EQUAL(offs, 0);

    offs = BINARY_SEARCH(entries, id, 43, entries_size);
    ASSERT_EQUAL(offs, 1);

    offs = BINARY_SEARCH(entries, id, 68, entries_size);
    ASSERT_EQUAL(offs, 1);

    offs = BINARY_SEARCH(entries, id, 70, entries_size);
    ASSERT_EQUAL(offs, 2);
}
