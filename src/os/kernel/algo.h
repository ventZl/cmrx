#pragma once

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
    unsigned lower = 0, upper = (_SIZE), pos = 0;\
    while (lower < upper) {\
        pos = lower + (upper - lower) / 2;\
        if (_ARRAY[pos]._KEY < _VALUE) {\
            lower = pos + 1;\
        } else {\
            upper = pos;\
        }\
    }\
    lower;\
})\

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
