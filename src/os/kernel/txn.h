#pragma once

#include <stdint.h>

/** @defgroup os_txn Transcation subsystem
 * @addtogroup os_kernel
 * CMRX kernel is mostly implemented as non-blocking. Where exclusivity is needed, transactions
 * are used in place of mutexes. Transactions provide a means of ensuring that the internal
 * state of program has been consistent thorough the execution of transaction.
 *
 * CMRX kernel provides two types of transactions:
 * * read-only transactions - these provide guarantee that the internal state of the system
 * did not change while the transaction was computing the result. If any modification to the
 * system state happened during the transaction, an attempt to commit read-only transaction will
 * fail.
 *
 * * read-write transactions - these provide means of gaining exclusive right to modify the
 * system state. When code is ready to perform the modification, it attempts to commit the
 * transaction. If there was no concurrent modification in between this transaction has been
 * created, then commit is granted an code can perform the modification in exclusive state.
 *
 * This approach trades better SMP capabilities and less contention for a bit of complexity
 * on the user side. Any code that uses transactions has to define what will happen if transaction
 * commit fails.
 *
 * For most use cases the way to deal with transaction commit failure is to redo the action
 * but some of them may figure out that the action is not needed anymore.
 *
 * @{ 
 */

/** Type of the transaction performed.
 */
enum TxnType {
    TXN_READONLY, ///< Read-only transaction
    TXN_READWRITE ///< Read-write transaction
};

typedef uint8_t Txn_t;

/** Start a transaction.
 * This function will start a transaction. Starting a transaction means that the code
 * will remember actual record version number (active rolling counter value). rolling
 * counters are updated whenever read-write transaction is commited.
 * @note It is perfectly valid to start transaction and not commit it. This effectively
 * equals to voluntarily aborting the transaction. Such abandonned transactions pose no 
 * overhead nor stall / corruption risk.
 * @return transaction ID
 */
Txn_t os_txn_start();

/** Try to commit the transaction.
 * This function will try to enter the critical section so that the code running 
 * afterwards has exclusive access to the shared resource.
 * If another transaction has been commited meanwhile, then this call will fail
 * as transaction couldn't be commited. Then the critical section is left
 * immediately.
 * If no other transaction conflicted with this one, the code remains in the critical
 * section and can modify the shared context.
 * After the code is done with modifying the shared context, it *must* call
 * @ref os_txn_done which will leave the critical section.
 * No modification of the shared context is allowed outside of the critical section.
 * @param [in] transaction the ID of transaction as obtained by the call to @ref ox_txn_start()
 * @param [in] type the type of transaction
 * @returns E_OK if transaction can be commited
 * @returns E_INVALID if transaction cannot be applied as different transaction has been
 * commited meanwhile.
 */
int os_txn_commit(Txn_t transaction, enum TxnType type);

/** Start new transaction and commit it immediately.
 * This is a combination of @ref os_txn_start and @ref os_txn_commit for special cases
 * where there is no intermediate computation needed. This is for cases, such as freeing
 * memory, releasing handles, etc. In these cases no search for free entry or any similar
 * action has to be done. Just a known-in-advance entry is freed and this change needs
 * to be transacted.
 * It will start new transaction and immediately commit it. As this is done atomically,
 * the commit never fails. This function always returns E_OK, which means that the code
 * is in commit mode, may change shared data and must call @ref os_txn_done when all 
 * changes are written.
 * @returns E_OK as this action never fails.
 */
int os_txn_start_commit();


/** Finish the transaction commit.
 * This function will leave the critical section related to the transaction.
 * This function has to be called for read-write transaction after all the data changes
 * in the transaction were commited.
 */
void os_txn_done();

/** @} */

