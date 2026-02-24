#include <ctest.h>
#include <cmrx/algo.h>
#include "algo_data.h"

#define BITMAP_SIZE     4
#define BITMAP_MAX      (BITMAP_SIZE * 8 * sizeof(uint32_t))

CTEST_DATA(algo_bitmap) {
    uint32_t bitmap[BITMAP_SIZE];
};

CTEST_SETUP(algo_bitmap) {
    for (unsigned q = 0; q < BITMAP_SIZE; ++q)
    {
        data->bitmap[q] = 0;
    }
}

CTEST2(algo_bitmap, find_empty) {
    uint32_t item = BITMAP_FIRST(data->bitmap, BITMAP_SIZE);
    ASSERT_EQUAL(item, ~0U);
}

CTEST2(algo_bitmap, set_one) {
    BITMAP_SET(data->bitmap, 0, BITMAP_MAX);
    bool is_set = BITMAP_TEST(data->bitmap, 0, BITMAP_SIZE);
    ASSERT_TRUE(is_set);
}

CTEST2(algo_bitmap, set_multiple) {
    BITMAP_SET(data->bitmap, 0, BITMAP_MAX);
    bool is_set = BITMAP_TEST(data->bitmap, 0, BITMAP_SIZE);
    ASSERT_TRUE(is_set);

    BITMAP_SET(data->bitmap, 1, BITMAP_MAX);
    is_set = BITMAP_TEST(data->bitmap, 1, BITMAP_SIZE);
    ASSERT_TRUE(is_set);

    BITMAP_SET(data->bitmap, 2, BITMAP_MAX);
    is_set = BITMAP_TEST(data->bitmap, 2, BITMAP_SIZE);
    ASSERT_TRUE(is_set);
}

/* Set last bit, check no previous was set */
CTEST2(algo_bitmap, set_and_check_correct_item) {
    BITMAP_SET(data->bitmap, BITMAP_MAX - 1, BITMAP_SIZE);
    uint32_t item = BITMAP_FIRST(data->bitmap, BITMAP_SIZE);
    ASSERT_EQUAL(item, BITMAP_MAX - 1);
}

/* Set multiple bits at the end of list, check no previous were set and no set bit was cleared */
CTEST2(algo_bitmap, set_multiple_check_correct) {
    BITMAP_SET(data->bitmap, BITMAP_MAX - 1, BITMAP_SIZE);

    BITMAP_SET(data->bitmap, BITMAP_MAX - 2, BITMAP_SIZE);

    uint32_t item = BITMAP_FIRST(data->bitmap, BITMAP_SIZE);
    ASSERT_EQUAL(item, BITMAP_MAX - 2);
    bool is_set = BITMAP_TEST(data->bitmap, BITMAP_MAX - 1, BITMAP_SIZE);
    ASSERT_TRUE(is_set);
}

CTEST2(algo_bitmap, set_and_clear_one) {
    BITMAP_SET(data->bitmap, 0, BITMAP_MAX);
    BITMAP_CLEAR(data->bitmap, 0, BITMAP_MAX);
    bool is_set = BITMAP_TEST(data->bitmap, 0, BITMAP_SIZE);
    ASSERT_FALSE(is_set);
}

CTEST2(algo_bitmap, set_and_clear_no_residues) {
    BITMAP_SET(data->bitmap, 0, BITMAP_MAX);
    BITMAP_CLEAR(data->bitmap, 0, BITMAP_MAX);

    uint32_t item = BITMAP_FIRST(data->bitmap, BITMAP_SIZE);
    ASSERT_EQUAL(item, ~0U);
}


CTEST2(algo_bitmap, set_and_clear_correct) {
    BITMAP_SET(data->bitmap, 0, BITMAP_MAX);
    BITMAP_SET(data->bitmap, 1, BITMAP_MAX);
    BITMAP_CLEAR(data->bitmap, 0, BITMAP_MAX);
    bool is_set = BITMAP_TEST(data->bitmap, 0, BITMAP_SIZE);
    ASSERT_FALSE(is_set);

    is_set = BITMAP_TEST(data->bitmap, 1, BITMAP_SIZE);
    ASSERT_TRUE(is_set);
}

CTEST2(algo_bitmap, set_copy_and_check) {
    uint32_t my_bitmap[BITMAP_SIZE];

    BITMAP_SET(data->bitmap, BITMAP_MAX - 1, BITMAP_SIZE);

    BITMAP_SET(data->bitmap, BITMAP_MAX - 2, BITMAP_SIZE);

    BITMAP_COPY(my_bitmap, data->bitmap, BITMAP_SIZE);

    uint32_t item = BITMAP_FIRST(my_bitmap, BITMAP_SIZE);
    ASSERT_EQUAL(item, BITMAP_MAX - 2);
    bool is_set = BITMAP_TEST(my_bitmap, BITMAP_MAX - 1, BITMAP_SIZE);
    ASSERT_TRUE(is_set);

}
