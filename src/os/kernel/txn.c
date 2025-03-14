#include <arch/corelocal.h>
#include "txn.h"
#include <cmrx/defines.h>
#include <cmrx/assert.h>
#include <conf/kernel.h>

#ifdef CMRX_KERNEL_TRANSACTION_VERIFICATION
static bool in_commit = false;

static inline void os_txn_mark_commit_start()
{
    in_commit = true;
}

static inline void os_txn_mark_commit_end()
{
    in_commit = false;
}

static inline void os_txn_assert_in_commit()
{
    ASSERT(in_commit);

}

static inline void os_txn_assert_not_in_commit()
{
    ASSERT(!in_commit);
}

#else

#define os_txn_mark_commit_start()
#define os_txn_mark_commit_end()
#define os_txn_assert_in_commit()
#define os_txn_assert_not_in_commit()

#endif

#include <stdbool.h>

static Txn_t os_txn_current_id = 0;

Txn_t os_txn_start() {
    os_core_lock();
    os_smp_lock();
    Txn_t rv = os_txn_current_id;
//    printf("Transaction started %u\n", rv);
    os_smp_unlock();
    os_core_unlock();

    return rv;
}

int os_txn_commit(Txn_t transaction, enum TxnType type) {
    os_core_lock();
    os_smp_lock();
    os_txn_assert_not_in_commit()
    if (os_txn_current_id == transaction) {
//        printf("Transaction committed %u\n", transaction);
        if (type == TXN_READWRITE) {
            os_txn_current_id++;
            os_txn_mark_commit_end();
            // critical section is entered by the calling code
        } else {
            // there is nothing to commit in read-only transaction so the critical
            // section is left
            os_smp_unlock();
            os_core_unlock();
        }
        
        return E_OK;
    } 

//    printf("Transaction aborted %u\n", transaction);
    // transaction must be aborted as another transaction has already been commited
    os_smp_unlock();
    os_core_unlock();
    return E_INVALID;
}

int os_txn_start_commit() {
    os_core_lock();
    os_smp_lock();
    os_txn_current_id++;
    os_txn_mark_commit_end();

    return E_OK;
}

void os_txn_done() {
    os_txn_assert_in_commit();
    os_txn_mark_commit_end();
    os_smp_unlock();
    os_core_unlock();

}
