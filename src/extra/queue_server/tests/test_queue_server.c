#include <ctest.h>
#include <extra/queue_server/queue.h>
#include <string.h>

static struct Queue queue;

#define TEST_QUEUE_DEPTH 16
#define TEST_QUEUE_ITEM_SIZE 12

int wait_for_object(void * addr, int timeout) {
    return 0;
}

CTEST_DATA(queue_server) {

};

CTEST_SETUP(queue_server) {
    bool rv = queue_init(&queue, TEST_QUEUE_DEPTH, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_empty_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE];
    bool rv = queue_receive(&queue, buffer);

    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_too_large_init_fail) {
    // make sure to initialize for too many items to be stored
    unsigned queue_items = (QUEUE_LENGTH * 2) / TEST_QUEUE_ITEM_SIZE;
    bool rv = queue_init(&queue, queue_items, TEST_QUEUE_ITEM_SIZE);

    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_write) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };

    bool rv = queue_send(&queue, buffer);

    ASSERT_EQUAL(rv, true);

}

CTEST2(queue_server, queue_write_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    unsigned char buffer2[TEST_QUEUE_ITEM_SIZE] = { 0 };

    int cmp = memcmp(buffer, buffer2, TEST_QUEUE_ITEM_SIZE);
    ASSERT_NOT_EQUAL(cmp, 0);

    bool rv = queue_send(&queue, buffer);

    ASSERT_EQUAL(rv, true);

    rv = queue_receive(&queue, buffer2);

    ASSERT_EQUAL(rv, true);

    cmp = memcmp(buffer, buffer2, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);
}

CTEST2(queue_server, queue_depth_correct) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    bool rv;
    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_send(&queue, buffer);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_max_depth_read) {
    unsigned char buffer_1st[TEST_QUEUE_ITEM_SIZE] = { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42 };
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };
    unsigned char buffer_out[TEST_QUEUE_ITEM_SIZE] = { 0 };
    bool rv;

    rv = queue_send(&queue, buffer_1st);
    ASSERT_EQUAL(rv, true);

    for (unsigned q = 1; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_receive(&queue, buffer_out);
    ASSERT_EQUAL(rv, true);

    int cmp = memcmp(buffer_1st, buffer_out, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);

    rv = queue_receive(&queue, buffer_out);
    ASSERT_EQUAL(rv, true);

    cmp = memcmp(buffer, buffer_out, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);
}

CTEST2(queue_server, queue_read_until_empty) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue, buffer);
        ASSERT_EQUAL(rv, true);
    }

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_receive(&queue, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_receive(&queue, buffer);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_initially) {
    bool rv = queue_empty(&queue);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_empty_after_write) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    rv = queue_send(&queue, buffer);

    rv = queue_empty(&queue);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_after_write_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    rv = queue_send(&queue, buffer);
    rv = queue_receive(&queue, buffer);

    rv = queue_empty(&queue);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_full_initially) {
    bool rv = queue_full(&queue);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_full_when_full) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue, buffer);
    }

    rv = queue_full(&queue);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_full_when_not_full) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue, buffer);
    }

    rv = queue_receive(&queue, buffer);

    rv = queue_full(&queue);
    ASSERT_EQUAL(rv, false);
}
