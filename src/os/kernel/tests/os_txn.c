#include <cmrx/os/txn.h>
#include <cmrx/defines.h>
#include <arch/corelocal.h>
#include <ctest.h>
#include <stdbool.h>>

extern callback_t cmrx_smp_wrong_lock_callback;

/* Undisturbed read-only transaction will commit cleanly */
CTEST(os_txn, txn_start_read_commit) 
{
    Txn_t id = os_txn_start();

    int rv = os_txn_commit(id, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);
}

/* Undisturbed read-write transaction will commit cleanly */
CTEST(os_txn, txn_start_write_commit) 
{
    Txn_t id = os_txn_start();

    int rv = os_txn_commit(id, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);
}

/* Succeeding undistrubed transactions commit
 * cleanly */
CTEST(os_txn, txn_serialize_write)
{
    Txn_t id = os_txn_start();

    int rv = os_txn_commit(id, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);

    id = os_txn_start();

    rv = os_txn_commit(id, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);
}

/* Succeeding undistrubed transactions commit
 * cleanly */
CTEST(os_txn, txn_serialize_read)
{
    Txn_t id = os_txn_start();

    int rv = os_txn_commit(id, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);

    id = os_txn_start();

    rv = os_txn_commit(id, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);
}

/* Succeeding undistrubed transactions commit
 * cleanly */
CTEST(os_txn, txn_serialize_mixed)
{
    Txn_t id = os_txn_start();

    int rv = os_txn_commit(id, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);

    id = os_txn_start();

    rv = os_txn_commit(id, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);

    id = os_txn_start();

    rv = os_txn_commit(id, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);
}

/* Commiting a read-only transaction won't invalidate
 * any on the fly read-only transaction. */
CTEST(os_txn, txn_multi_read_commit_clean)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_2, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_1, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);
}

/* Commiting a read-only transaction won't invalidate
 * any on the fly read-only transaction. */
CTEST(os_txn, txn_read_interleaved_commit_clean)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_1, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_2, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);
}

/* Commiting a read-only transaction won't invalidate
 * any on the fly read-write transaction. */
CTEST(os_txn, txn_write_interrupt_read_commit_clean)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_2, TXN_READONLY);

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_1, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);
}

/* Commiting a read-write transaction will abort all
 * on the fly read-only transaction. */
CTEST(os_txn, txn_read_write_interleaved_abort)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_1, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_2, TXN_READONLY);

    ASSERT_EQUAL(rv, E_INVALID);
}

/* Commiting a read-write transaction will abort
 * all on the fly read-write transactions. */
CTEST(os_txn, txn_multi_write_aborted)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_2, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_1, TXN_READWRITE);

    ASSERT_EQUAL(rv, E_INVALID);
}

/* Commiting a read-write transaction will abort all
 * on the fly read-only transaction. */
CTEST(os_txn, txn_read_nested_write_aborted)
{
    Txn_t id_1 = os_txn_start();
    Txn_t id_2 = os_txn_start();

    int rv = os_txn_commit(id_2, TXN_READWRITE);
    os_txn_done();

    ASSERT_EQUAL(rv, E_OK);

    rv = os_txn_commit(id_1, TXN_READONLY);

    ASSERT_EQUAL(rv, E_INVALID);
}

static bool test_lock_abort_called = false;

static void test_lock_aborted() {
    test_lock_abort_called = true;
}

/* Transaction creation cannot be nested into commit
 * critical section */
CTEST(os_txn, commit_neste_start_detected)
{
    cmrx_smp_wrong_lock_callback = test_lock_aborted;

    test_lock_abort_called = false;

    Txn_t id_1 = os_txn_start();

    ASSERT_EQUAL(test_lock_abort_called, false);

    int rv = os_txn_commit(id_1, TXN_READWRITE);
    
    ASSERT_EQUAL(test_lock_abort_called, false);

    Txn_t id_2 = os_txn_start();

    ASSERT_EQUAL(test_lock_abort_called, true);
    
    os_txn_done();
}
