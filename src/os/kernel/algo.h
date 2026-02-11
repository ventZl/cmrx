#pragma once

#include <stdint.h>

/** Binary search algorithm.
 * This is a template for binary searching over a sorted compacted array.
 * It will search either for exact match or in case the sought key is not
 * present in dataset, for lower bounds - entry present in array sorting
 * just before the sought entry.
 *
 * As with any binary search, this algorithm assumes that data is sorted.
 * This template expects data to be stored in array of structures with
 * structure containing member holding item's unique ID. This ID is used
 * for searching.
 *
 * Example:
 *
 * ~~~
 * typedef struct {
 *     unsigned id;
 *     unsigned data;
 * } Entry_t;
 *
 * Entry_t entries[ENTRIES_MAX];
 * unsigned entries_size;
 *
 * ...
 *
 * unsigned offs = BINARY_SEARCH(entries, id, 42, entries_size);
 * ~~~
 *
 * @param _ARRAY identification of array which is used to search for entry
 * @param _KEY name of structure member which holds unique ID of entry
 * @param _VALUE unique ID sought for in the array
 * @param _SIZE amount of valid entries currently stored in the array
 * @returns array offset of entry with matching ID or with ID which is nearest
 * smaller than ID sought for, if sought-for ID is not present.
 */

#define BINARY_SEARCH(_ARRAY, _KEY, _VALUE, _SIZE) \
({\
    unsigned lower = 0, upper = (_SIZE);\
    /* Binary search until range is small */\
    while (upper - lower > 8) {\
        unsigned mid = lower + ((upper - lower) >> 1);\
        if (_ARRAY[mid]._KEY < _VALUE)\
            lower = mid + 1;\
            else\
                upper = mid;\
    }\
    /* Linear scan for final elements */\
    while (lower < upper && _ARRAY[lower]._KEY < _VALUE) {\
        lower++;\
    }\
    lower;\
})

/** Insert into array algorithm.
 * This template will create free space in array, move all entries after the
 * place of insertion one entry higher and increase array size.
 * @param _ARRAY array being inserted into
 * @param _POS position at which insertion happens
 * @param _SIZE lvalue holding current array size (e.g. count of actual stored
 * items rather than allocation size)
 * @note This template does not check for array overflowing
 */
#define ARRAY_INSERT(_ARRAY, _POS, _SIZE) \
    for (unsigned q = _SIZE; q > _POS; --q)\
    {\
        _ARRAY[q] = _ARRAY[q - 1];\
    }\
    _SIZE++;

/** Delete from array algorithm.
 * This template will delete item from array, move all entries after the
 * place of insertion one entry lower and decrease array size.
 * @param _ARRAY array being inserted into
 * @param _POS position at which deletion happens
 * @param _SIZE lvalue holding current array size (e.g. count of actual stored
 * items rather than allocation size)
 */
#define ARRAY_DELETE(_ARRAY, _POS, _SIZE) \
    for (unsigned q = _POS + 1; q < _SIZE; ++q)\
    {\
        _ARRAY[q - 1] = _ARRAY[q];\
    }\
    _SIZE--;

/** Allocate space for inserting item into sorted array.
 * This template will allocate space for inserting new item into sorted
 * array so that it stays sorted. It will do so that the array remains
 * sorted after the insertion. Template code will move array contents
 * around to make space for new entry but won't write it. That's caller
 * responsibility.
 *
 * This algorithm assumes that identifiers of items in array are unique.
 * Thus, if ID passed for new entry already exists in the array, it will
 * be overwritten by subsequent call.
 *
 * Example:
 *
 * ~~~
 * typedef struct {
 *     unsigned id;
 *     unsigned data;
 * } Entry_t;
 *
 * Entry_t entries[ENTRIES_MAX];
 * unsigned entries_size;
 *
 * ...
 *
 * Entry_t new_entry = { .id = 42, .data = 0xBABEDEAD };
 * unsigned offs = INSERT_SORT(entries, id, new_entry.id, entries_size, ENTRIES_MAX);
 * entries[offs] = new_entry;
 * ~~~
 *
 * Note that the algorithm will avoid corrupting memory if there is
 * a request to insert new entry into already-full array but won't return
 * any special return value. It's caller's responsibility to check if array
 * is already full before making the call.
 *
 * This algorithm works in a way that if ID of new entry already exists in
 * the array, array content won't be moved around and subsequent write will
 * overwrite the existing entry. If ID doesn't exist, space for it will be
 * allocated by moving items around the array so the array remains sorted
 * after insertion.
 *
 * @param _ARRAY identification of array which is used to search for entry
 * @param _KEY name of structure member which holds unique ID of entry
 * @param _VALUE unique ID sought for in the array
 * @param _SIZE amount of valid entries currently stored in the array.
 * This argument must be a lvalue as it will be modified during the process.
 * @param _MAX capacity of the array.
 * @returns array offset where new entry may be inserted. This position is always
 * valid but care has to be taken to check if array capacity is not exhausted.
 * Caller should check that _SIZE < _MAX upon insertion, if entry with given
 * ID didn't exist before.
 */
#define INSERT_SORT(_ARRAY, _KEY, _VALUE, _SIZE, _MAX) \
({\
    unsigned pos = BINARY_SEARCH(_ARRAY, _KEY, _VALUE, _SIZE);\
    if (((_ARRAY[pos]._KEY != _VALUE) || (pos == _SIZE)) \
        && (_SIZE < _MAX)) \
    {\
        for (unsigned q = _SIZE; q > pos; --q)\
        {\
            _ARRAY[q] = _ARRAY[q - 1];\
        }\
        _SIZE++;\
    }\
    pos;\
})

#ifndef CMRX_USE_FAST_HASH
/* Prospector-generated hash function.
 * Source for constants: https://github.com/skeeto/hash-prospector/issues/19 */
inline uint32_t os_hash_key(uint32_t key) {
    key ^= key >> 16;
    key *= 0x21f0aaadU;
    key ^= key >> 15;
    key *= 0xf35a2d97U;
    key ^= key >> 15;
    return key;
}
#else
/* Minimal mixer */
inline uint32_t os_hash_key(uint32_t key)
{
    key ^= key >> 16;
    key *= 0x45D9F3BU;
    key ^= key >> 16;
    return key;
}
#endif

#define HASH_EMPTY 0xFFFFFFFFU

/** Find entry in hash table.
 * This is a template for searching hash tables. It either finds the entry
 * if it is present in the table or returns position of an entry that is
 * marked as empty.
 *
 * Example:
 * ~~~
 * typedef struct {
 *     unsigned id;
 *     unsigned data;
 * } Entry_t;
 *
 * Entry_t hash_table[ENTRIES_MAX];
 *
 * ...
 *
 * Entry_t new_entry = { .id = 42, .data = 0xBABEDEAD };
 * unsigned offs = HASH_SEARCH(hash_table, id, new_entry.id, ENTRIES_MAX);
 * entries[offs] = new_entry;
 * ~~~
 *
 * @note Key must not have a value of @ref HASH_EMPTY which is a reserved value
 * used to denote that the slot is empty.
 *
 * @warning This algorithm doesn't deal with hash table full status. In general it
 * is recommended to keep hash tables at 50-70% load. If load exceeds these
 * values performance will start dropping yet hash function should still succeed.
 * To avoid infinite cycle when trying to insert new with key not present in hash
 * table while the hash table is full, an external check for hash table load
 * must be performed.
 *
 * @param _HASHTABLE identification of the hash table where search is performed
 * @param _KEY name of structure member which holds unique ID of entry
 * @param _VALUE unique ID sought for in the hash table
 * @param _MAX allocation size of the hash table
 * @returns offset in the hash table which either contains the entry sought for
 * or an empty slot.
 */
#define HASH_SEARCH(_HASHTABLE, _KEY, _VALUE, _MAX) \
({\
    _Static_assert(((_MAX) & ((_MAX) - 1)) == 0 && (_MAX) != 0, "HASH_SEARCH: _MAX must be a power of 2");\
    const uint32_t hash = os_hash_key((uint32_t)_VALUE);\
    const uint32_t mask = (_MAX) - 1;\
    uint32_t pos = hash & mask;\
    uint32_t stride = 1;\
    while (_HASHTABLE[pos]._KEY != _VALUE && _HASHTABLE[pos]._KEY != HASH_EMPTY) {\
        pos = (pos + stride) & mask;\
        stride++;\
    }\
    pos;\
})

#define BITMAP_SET(_BITMAP, _POS, _SIZE) _BITMAP[(_POS) >> 5] |= (1U << ((_POS) & 31));
#define BITMAP_CLEAR(_BITMAP, _POS, _SIZE) _BITMAP[(_POS) >> 5] &= ~(1U << ((_POS) & 31));
#define BITMAP_TEST(_BITMAP, _POS, _SIZE) return (_BITMAP[(_POS) >> 5] & (1U << ((_POS) & 31))) != 0;
#define BITMAP_FIRST(_BITMAP, _SIZE) \
({\
    uint32_t first = ~0;\
    for (int word = 0; word < _SIZE; ++word) {\
        uint32_t ready = _BITMAP[word];\
        if (ready) {\
            uint8_t bit = __builtin_ctz(ready);\
            first = (word << 5) + bit;\
            break;\
        }\
    }\
    first;\
})
#define BITMAP_COPY(_TARGET, _SOURCE, _SIZE) \
    for (unsigned q = 0; q < _SIZE; ++q) _TARGET[q] = _SOURCE[q];
