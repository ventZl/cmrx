#include <ctest.h>
#include <extra/queue_server/queue.h>
#include <string.h>
#include <cmrx/defines.h>

STATIC_QUEUE(queue, 256);

#define TEST_QUEUE_DEPTH 16
#define TEST_QUEUE_ITEM_SIZE 12

static bool wait_for_object_called = false;
static void * wait_for_object_addr = NULL;
/// How long will it take to notification to arrive?
/// If set to 0 (default), it will arrive immediately
static uint32_t wait_for_object_notified_after = 0;
static void (*wait_for_object_notified_worker_func)() = NULL;

static bool notify_object_called = false;
static void * notify_object_addr = NULL;

int notify_object(void * addr) {
    notify_object_called = true;
    notify_object_addr = addr;
    return 0;
}

int wait_for_object(void * addr, uint32_t timeout) {
    wait_for_object_called = true;
    wait_for_object_addr = addr;
    if (timeout == 0)
    {
        return E_OK;
    }
    else
    {
        if (timeout > wait_for_object_notified_after)
        {
            if (wait_for_object_notified_worker_func != NULL)
            {
                wait_for_object_notified_worker_func();
            }
            else
            {
                // Asserted here? Your test is wrong! You need to pass a function
                // to this variable that gets called to simulate late arriving
                // notification!
                ASSERT_NOT_EQUAL((unsigned long) wait_for_object_notified_worker_func, 0);
            }
            return E_OK;
        }
        else
        {
            return E_TIMEOUT;
        }
    }
}

CTEST_DATA(queue_server) {

};

CTEST_SETUP(queue_server) {
    wait_for_object_called = false;
    notify_object_called = false;
    wait_for_object_addr = NULL;
    notify_object_addr = NULL;
    bool rv = queue_init(&queue.q, TEST_QUEUE_DEPTH, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_empty_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE];

    // Precondition check
    ASSERT_EQUAL(wait_for_object_called, false);

    bool rv = queue_receive(&queue.q, buffer);

    ASSERT_EQUAL(rv, false);
    ASSERT_EQUAL(wait_for_object_called, true);
}

// Test skipped as currently this cannot be reasonably triggered
CTEST2_SKIP(queue_server, queue_too_large_init_fail) {
    // make sure to initialize for too many items to be stored
    bool rv = queue_init(&queue.q, TEST_QUEUE_DEPTH * 2, TEST_QUEUE_ITEM_SIZE);

    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_write) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    ASSERT_EQUAL(notify_object_called, false);

    bool rv = queue_send(&queue.q, buffer);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(notify_object_called, true);
}

CTEST2(queue_server, queue_empty_silent_write) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    ASSERT_EQUAL(notify_object_called, false);

    bool rv = queue_send_silent(&queue.q, buffer);

    ASSERT_EQUAL(rv, true);
    ASSERT_EQUAL(notify_object_called, false);

}

CTEST2(queue_server, queue_write_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    unsigned char buffer2[TEST_QUEUE_ITEM_SIZE] = { 0 };

    int cmp = memcmp(buffer, buffer2, TEST_QUEUE_ITEM_SIZE);
    ASSERT_NOT_EQUAL(cmp, 0);

    bool rv = queue_send(&queue.q, buffer);

    ASSERT_EQUAL(notify_object_called, true);
    ASSERT_EQUAL(rv, true);

    rv = queue_receive(&queue.q, buffer2);

    ASSERT_EQUAL(wait_for_object_called, false);
    ASSERT_EQUAL(rv, true);

    cmp = memcmp(buffer, buffer2, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);
}

CTEST2(queue_server, queue_depth_correct) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0x42 };
    bool rv;
    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue.q, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_send(&queue.q, buffer);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_max_depth_read) {
    unsigned char buffer_1st[TEST_QUEUE_ITEM_SIZE] = { 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42 };
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };
    unsigned char buffer_out[TEST_QUEUE_ITEM_SIZE] = { 0 };
    bool rv;

    rv = queue_send(&queue.q, buffer_1st);
    ASSERT_EQUAL(rv, true);

    for (unsigned q = 1; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue.q, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_receive(&queue.q, buffer_out);
    ASSERT_EQUAL(rv, true);

    int cmp = memcmp(buffer_1st, buffer_out, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);

    rv = queue_receive(&queue.q, buffer_out);
    ASSERT_EQUAL(rv, true);

    cmp = memcmp(buffer, buffer_out, TEST_QUEUE_ITEM_SIZE);
    ASSERT_EQUAL(cmp, 0);
}

CTEST2(queue_server, queue_read_until_empty) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue.q, buffer);
        ASSERT_EQUAL(rv, true);
    }

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_receive(&queue.q, buffer);
        ASSERT_EQUAL(rv, true);
    }

    rv = queue_receive(&queue.q, buffer);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_initially) {
    bool rv = queue_empty(&queue.q);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_empty_after_write) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    rv = queue_send(&queue.q, buffer);

    rv = queue_empty(&queue.q);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_empty_after_write_read) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    rv = queue_send(&queue.q, buffer);
    rv = queue_receive(&queue.q, buffer);

    rv = queue_empty(&queue.q);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_full_initially) {
    bool rv = queue_full(&queue.q);
    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_full_when_full) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue.q, buffer);
    }

    rv = queue_full(&queue.q);
    ASSERT_EQUAL(rv, true);
}

CTEST2(queue_server, queue_not_full_when_not_full) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    bool rv;

    for (unsigned q = 0; q < TEST_QUEUE_DEPTH; ++q) {
        rv = queue_send(&queue.q, buffer);
    }

    rv = queue_receive(&queue.q, buffer);

    rv = queue_full(&queue.q);
    ASSERT_EQUAL(rv, false);
}

static void queue_timeout_late_data_arrival()
{
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    (void) queue_send(&queue.q, buffer);
}

static void queue_timeout_no_data_arrival()
{
    // this function intentionally left blank
}

CTEST2(queue_server, queue_timeout_times_out_with_data) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    // Fake notification to be received after 50ms
    wait_for_object_notified_after = 50000;
    wait_for_object_notified_worker_func = queue_timeout_late_data_arrival;

    // Wait for the notification for 25ms
    int rv = queue_receive_timeout(&queue.q, buffer, 25000);

    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_timeout_times_out_no_data) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    // Fake notification to be received after 50ms
    wait_for_object_notified_after = 50000;
    wait_for_object_notified_worker_func = queue_timeout_no_data_arrival;

    // Wait for the notification for 25ms
    int rv = queue_receive_timeout(&queue.q, buffer, 25000);

    ASSERT_EQUAL(rv, false);
}

CTEST2(queue_server, queue_timeout_data_arrives) {
    unsigned char buffer[TEST_QUEUE_ITEM_SIZE] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };

    // Fake notification to be received after 25ms
    wait_for_object_notified_after = 25000;
    wait_for_object_notified_worker_func = queue_timeout_late_data_arrival;

    // Wait for the notification for 50ms
    int rv = queue_receive_timeout(&queue.q, buffer, 50000);

    ASSERT_EQUAL(rv, true);
}
