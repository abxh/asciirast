/**
 * @file hashtable.h
 * @brief Hashtable data structure based on robin hood hashing and dynamic array.
 */

#include "is_pow2.h"    // is_pow2
#include "murmurhash.h" // murmur3_32

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @def HASHTABLE_PREFIX
 * @brief The hashtable prefix. Is undefined once header is included.
 * @attention This must be manually defined before including this header file.
 */
#ifndef HASHTABLE_PREFIX
#error "Must declare HASHTABLE_PREFIX. Defaulting to hashtable."
#define HASHTABLE_PREFIX hashtable
#endif

/**
 * @def KEY_TYPE
 * @brief The key type. Is undefined once header is included.
 * @attention This must be manually defined before including this header file.
 *
 */
#ifndef KEY_TYPE
#error "Must declare KEY_TYPE. Defaulting to int."
#define KEY_TYPE int
#endif

/**
 * @def VALUE_TYPE
 * @brief The value type. Is undefined once header is included.
 * @attention This must be manually defined before including this header file.
 */
#ifndef VALUE_TYPE
#error "Must declare VALUE_TYPE. Defaulting to int."
#define VALUE_TYPE int
#endif

/**
 * @def KEY_IS_EQUAL(a, b)
 * @brief Used to compare two keys. Is undefined once header is included.
 * @attention
 *   @li If comparing two primtive values, define KEY_IS_EQUAL(a,b) to ((a) == (b)).
 *   @li If comparing two structs, set this macro to a function that does element-wise comparison between the structs.
 *   @li If comparing two strings, use strcmp or wrap the string in a struct with string length info and define KEY_IS_EQUAL to a
 *       unwrapper function that in turn calls strncmp.
 *
 * @retval true If the two keys are equal. Equivalent to a non-zero int.
 * @retval false If the two key are not equal. Equivalent to the int 0.
 */
#ifndef KEY_IS_EQUAL
#error "Must declare KEY_IS_EQUAL. Defaulting to ((a) == (b))."
#define KEY_IS_EQUAL(a, b) ((a) == (b))
#endif

/**
 * @def HASH_FUNCTION(key)
 * @brief Used to compute the hash of a key and locate it's index in the hashtable. Is undefined once header is included.
 * @note This is defaulted to use murmur3 hash.
 * @param key The key at hand to be converted to bytes.
 * @retval The hash of the key able to be stored in `size_t`.
 */
#ifndef HASH_FUNCTION
#define HASH_FUNCTION(key) (murmur3_32((const uint8_t*)&(key), sizeof(KEY_TYPE), 0))
#endif

/**
 * @def EMPTY_HASHTABLE_SLOT_OFFSET
 * @brief Offset constant used to flag empty slots.
 */
#ifndef EMPTY_HASHTABLE_SLOT_OFFSET
#define EMPTY_HASHTABLE_SLOT_OFFSET (SIZE_MAX)
#endif

/**
 * @def hashtable_for_each
 * @brief Iterate over the non-empty slots in the hashtable in arbitary order.
 * @param[in] hashtable_ptr hashtable pointer.
 * @param[out] index The index of the value in the hashtable.
 * @param[out] key Key at `index` as `KEY_TYPE`.
 * @param[out] value Value at `index` as `VALUE_TYPE`.
 */
#ifndef hashtable_for_each
#define hashtable_for_each(hashtable_ptr, index, key, value)                         \
    for ((index) = 0; (index) <= (hashtable_ptr)->index_mask; (index)++)             \
        if ((hashtable_ptr)->slots[(index)].offset != EMPTY_HASHTABLE_SLOT_OFFSET && \
            ((key) = (hashtable_ptr)->slots[(index)].key, (value) = (hashtable_ptr)->slots[(index)].value, true))
#endif

/// @cond DO_NOT_DOCUMENT
// helper macros:
#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between
/// @endcond

/* The hashtable type name. Set to HASHTABLE_PREFIX##_##type. */
#ifndef HASHTABLE_TYPE
#define HASHTABLE_TYPE JOIN(HASHTABLE_PREFIX, type)
#endif

/* The hashtable slot type name. Set to HASHTABLE_PREFIX##_slot_type. */
#ifndef HASHTABLE_SLOT_TYPE
#define HASHTABLE_SLOT_TYPE JOIN(HASHTABLE_PREFIX, slot_type)
#endif

/**
 * @brief Generated hashtable slot struct type for a given key and value type.
 */
typedef struct {
    size_t offset;    ///< Offset from the ideal slot index.
    KEY_TYPE key;     ///< The key in this slot
    VALUE_TYPE value; ///< The value in this slot
} HASHTABLE_SLOT_TYPE;

/**
 * @brief Generated hashtable struct type for a given key and value type.
 */
typedef struct {
    size_t count;               ///< number of non-empty slots.
    size_t index_mask;          ///< the mask used to wrap indicies around the array of slots.
    HASHTABLE_SLOT_TYPE* slots; ///< pointer to array containing the slots in the hashtable.
} HASHTABLE_TYPE;

/**
 * @brief Clear an existing hashtable and flag all slots as empty.
 * @param[in] hashtable_ptr The pointer of the hashtable to clear.
 *
 * If `hashtable_ptr` is `NULL` then nothing is done.
 */
static inline void JOIN(HASHTABLE_PREFIX, clear)(HASHTABLE_TYPE* hashtable_ptr) {
    if (!hashtable_ptr) {
        return;
    }
    size_t capacity = hashtable_ptr->index_mask + 1;
    for (size_t i = 0; i < capacity; i++) {
        hashtable_ptr->slots[i].offset = EMPTY_HASHTABLE_SLOT_OFFSET;
    }
}

/**
 * @brief Create a hashtable with specified initial power of two capacity.
 *
 * @param[in] initial_pow2_capacity The initial capacity of the hashtable, which must be a power of 2 and not 1.
 * @return Pointer to the created hashtable.
 * @retval `NULL`
 *   @li If `initial_pow2_capacity` is not a power of 2 or is equal to 1.
 *   @li If `initial_pow2_capacity * sizeof(K_V_slot_type)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline HASHTABLE_TYPE* JOIN(HASHTABLE_PREFIX, create_with_initial_capacity)(const size_t initial_pow2_capacity) {
    if (!is_pow2(initial_pow2_capacity) || initial_pow2_capacity - 1 == 0 || initial_pow2_capacity > SIZE_MAX / sizeof(VALUE_TYPE)) {
        return NULL;
    }
    HASHTABLE_TYPE* hashtable_ptr = malloc(sizeof(HASHTABLE_TYPE));
    if (!hashtable_ptr) {
        return NULL;
    }
    hashtable_ptr->slots = malloc(sizeof(HASHTABLE_SLOT_TYPE) * initial_pow2_capacity);
    if (!hashtable_ptr->slots) {
        free(hashtable_ptr);
        return NULL;
    }
    hashtable_ptr->count = 0;
    hashtable_ptr->index_mask = initial_pow2_capacity - 1;

    JOIN(HASHTABLE_PREFIX, clear)(hashtable_ptr);

    return hashtable_ptr;
}

/**
 * @brief Create a hashtable with a default capacity (512).
 *
 * @return The hashtable pointer.
 * @retval `NULL` If no memory space is available.
 */
static inline HASHTABLE_TYPE* JOIN(HASHTABLE_PREFIX, create)(void) {
    return JOIN(HASHTABLE_PREFIX, create_with_initial_capacity)(512);
}

/**
 * @brief Destroy a hashtable and free the used memory.
 *
 * @warning May not be called twice in a row on the same hashtable.
 */
static inline void JOIN(HASHTABLE_PREFIX, destroy)(HASHTABLE_TYPE* hashtable_ptr) {
    if (!hashtable_ptr) {
        return;
    }
    free(hashtable_ptr->slots);
    free(hashtable_ptr);
}

/**
 * @brief Clone an existing hashtable.
 *
 * @param[in] hashtable_ptr The pointer of the hashtable to clone.
 * @return A pointer to the clone of the original hashtable.
 * @retval `NULL`
 *   @li If no memory space is available.
 *   @li If hashtable_ptr is `NULL`.
 */
static inline HASHTABLE_TYPE* JOIN(HASHTABLE_PREFIX, clone)(const HASHTABLE_TYPE* hashtable_ptr) {
    if (!hashtable_ptr) {
        return NULL;
    }
    const size_t capacity = hashtable_ptr->index_mask + 1;

    HASHTABLE_TYPE* other_hashtable_ptr = JOIN(HASHTABLE_PREFIX, create_with_initial_capacity)(capacity);
    if (!other_hashtable_ptr) {
        return NULL;
    }

    for (size_t i = 0; i < capacity; i++) {
        other_hashtable_ptr->slots[i] = hashtable_ptr->slots[i];
    }

    other_hashtable_ptr->count = hashtable_ptr->count;

    return other_hashtable_ptr;
}

/**
 * @brief Get the number of non-empty slots in the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @return The number of non-empty slots as `size_t`.
 */
static inline size_t JOIN(HASHTABLE_PREFIX, count)(const HASHTABLE_TYPE* hashtable_ptr) {
    assert(NULL != hashtable_ptr);

    return hashtable_ptr->count;
}

/**
 * @brief Get the number of slots allocated for in the hashtable. Capacity is grown as needed.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @return The current capacity as `size_t`.
 */
static inline size_t JOIN(HASHTABLE_PREFIX, capacity)(const HASHTABLE_TYPE* hashtable_ptr) {
    assert(NULL != hashtable_ptr);

    return hashtable_ptr->index_mask + 1;
}

/**
 * @brief Check if hashtable contains at least one non-empty slot.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @return A boolean indicating whether the hashtable is empty.
 */
static inline bool JOIN(HASHTABLE_PREFIX, is_empty)(const HASHTABLE_TYPE* hashtable_ptr) {
    assert(NULL != hashtable_ptr);

    return hashtable_ptr->count == 0;
}

/**
 * @brief Check if hashtable contains a key.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key.
 * @return A boolean indicating whether the hashtable contains the given key.
 */
static inline bool JOIN(HASHTABLE_PREFIX, contains_key)(const HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key) {
    assert(NULL != hashtable_ptr);

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    size_t max_possible_offset = 0;

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET &&
           max_possible_offset <= hashtable_ptr->slots[index].offset) {
        if (KEY_IS_EQUAL(hashtable_ptr->slots[index].key, key)) {
            return true;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        max_possible_offset++;
    }
    return false;
}

/**
 * @brief From a given key, get the pointer to the corresponding value in the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * The returned pointer is **not** garanteed to point to the same value if the hashtable
 * is modified.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key to search for.
 * @return A pointer to the corresponding key.
 * @retval NULL If the hashtable did not contain the key.
 */
static inline VALUE_TYPE* JOIN(HASHTABLE_PREFIX, get_mut)(const HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key) {
    assert(NULL != hashtable_ptr);

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    size_t max_possible_offset = 0;

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET &&
           max_possible_offset <= hashtable_ptr->slots[index].offset) {
        if (KEY_IS_EQUAL(hashtable_ptr->slots[index].key, key)) {
            return &hashtable_ptr->slots[index].value;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        max_possible_offset++;
    }
    return NULL;
}

/**
 * @brief From a given key, get the copy of the corresponding value in the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key to search for.
 * @param[in] default_value The default value returned if the hashtable did not contain the key.
 * @return The corresponding key.
 * @retval default_value If the hashtable did not contain the key.
 */
static inline VALUE_TYPE JOIN(HASHTABLE_PREFIX, get)(const HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key,
                                                     const VALUE_TYPE default_value) {
    assert(NULL != hashtable_ptr);

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    size_t max_possible_offset = 0;

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET &&
           max_possible_offset <= hashtable_ptr->slots[index].offset) {
        if (KEY_IS_EQUAL(hashtable_ptr->slots[index].key, key)) {
            return hashtable_ptr->slots[index].value;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        max_possible_offset++;
    }
    return default_value;
}

/**
 * @brief Search a given key in the hashtable and get the pointer to the corresponding value.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * The returned pointer is **not** garanteed to point to the same value if the hashtable
 * is modified.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key to search for.
 * @return A pointer to the corresponding key.
 * @retval NULL If the hashtable did not contain the key.
 */
static inline VALUE_TYPE* JOIN(HASHTABLE_PREFIX, search)(HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key) {
    return JOIN(HASHTABLE_PREFIX, get_mut)(hashtable_ptr, key);
}

/**
 * @brief Resize and grow the hashtable slots array manually.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] new_capacity The new capacity.
 * @return A boolean indicating whether the hashtable was able to be resized to grow.
 * @retval false
 *   @li If `new_capacity` is not a power of two.
 *   @li If `new_capacity` is smaller than or equal to the current capacity.
 *   @li If `new_capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline bool JOIN(HASHTABLE_PREFIX, grow)(HASHTABLE_TYPE* hashtable_ptr, const size_t new_capacity) {
    assert(NULL != hashtable_ptr);

    const size_t old_capacity = hashtable_ptr->index_mask + 1;
    if (!is_pow2(new_capacity) || new_capacity <= old_capacity || new_capacity > SIZE_MAX / sizeof(VALUE_TYPE)) {
        return false;
    }

    HASHTABLE_SLOT_TYPE* new_slots = malloc(sizeof(HASHTABLE_SLOT_TYPE) * new_capacity);
    if (new_slots == NULL) {
        return false;
    }

    for (size_t i = 0; i < new_capacity; i++) {
        new_slots[i].offset = EMPTY_HASHTABLE_SLOT_OFFSET;
    }

    const size_t new_index_mask = new_capacity - 1;

    for (size_t index_at_old = 0; index_at_old < old_capacity; (index_at_old)++) {
        if (hashtable_ptr->slots[index_at_old].offset == EMPTY_HASHTABLE_SLOT_OFFSET) {
            continue;
        }
        // insertion algorithm - modified for rehashing.
        const KEY_TYPE key = hashtable_ptr->slots[index_at_old].key;
        const VALUE_TYPE value = hashtable_ptr->slots[index_at_old].value;

        const size_t key_hash = (size_t)HASH_FUNCTION(key);

        size_t index_at_new = key_hash & new_index_mask;
        HASHTABLE_SLOT_TYPE current_slot = {.offset = 0, .key = key, .value = value};

        while (new_slots[index_at_new].offset != EMPTY_HASHTABLE_SLOT_OFFSET) {
            if (current_slot.offset > new_slots[index_at_new].offset) {
                const HASHTABLE_SLOT_TYPE temp = new_slots[index_at_new];
                new_slots[index_at_new] = current_slot;
                current_slot = temp;
            }
            index_at_new = (index_at_old + 1) & new_index_mask;
            current_slot.offset++;
        }
        new_slots[index_at_new] = current_slot;
    }

    free(hashtable_ptr->slots);

    hashtable_ptr->slots = new_slots;
    hashtable_ptr->index_mask = new_index_mask;

    return true;
}

/**
 * @brief Insert a non-duplicate key and it's corresponding value inside the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL` and key is not already contained in the hashtable.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key.
 * @param[in] value The value.
 * @return A boolean indicating whether the key-value pair was stored, depending on
 *         whether the hashtable could be resized once load factor is 0.5.
 * @retval false
 *   @li If `2 * current capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline bool JOIN(HASHTABLE_PREFIX, insert)(HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key, const VALUE_TYPE value) {
    assert(NULL != hashtable_ptr);
    assert(false == JOIN(HASHTABLE_PREFIX, contains_key)(hashtable_ptr, key));

    const size_t old_capacity = hashtable_ptr->index_mask + 1;
    if (hashtable_ptr->count >= old_capacity / 2 && !JOIN(HASHTABLE_PREFIX, grow)(hashtable_ptr, 2 * old_capacity)) {
        return false;
    }

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    HASHTABLE_SLOT_TYPE current_slot = {.offset = 0, .key = key, .value = value};

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET) {
        // swap if current offset is larger. will ensure the maximum
        // offset (from the ideal position) is minimized.
        if (current_slot.offset > hashtable_ptr->slots[index].offset) {
            const HASHTABLE_SLOT_TYPE temp = hashtable_ptr->slots[index];
            hashtable_ptr->slots[index] = current_slot;
            current_slot = temp;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        current_slot.offset++;
    }
    hashtable_ptr->slots[index] = current_slot;
    hashtable_ptr->count++;

    return true;
}

/**
 * @brief Update a key's corresponding value inside the hashtable.
 *
 * Inserts the key-value pair if the key is not already contained in the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key.
 * @param[in] value The value.
 * @return A boolean indicating whether the key-value pair was stored, depending on
 *         whether the hashtable could be resized once load factor is 0.5.
 * @retval false
 *   @li If `2 * current capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline bool JOIN(HASHTABLE_PREFIX, update)(HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key, const VALUE_TYPE value) {
    assert(NULL != hashtable_ptr);

    const size_t old_capacity = hashtable_ptr->index_mask + 1;
    if (hashtable_ptr->count >= old_capacity / 2 && !JOIN(HASHTABLE_PREFIX, grow)(hashtable_ptr, 2 * old_capacity)) {
        return false;
    }

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    HASHTABLE_SLOT_TYPE current_slot = {.offset = 0, .key = key, .value = value};

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET) {
        if (current_slot.offset == hashtable_ptr->slots[index].offset &&
            KEY_IS_EQUAL(current_slot.key, hashtable_ptr->slots[index].key)) {
            hashtable_ptr->slots[index].value = current_slot.value;

            return true;
        }
        // swap if current offset is larger. will ensure the maximum
        // offset (from the ideal position) is minimized.
        if (current_slot.offset > hashtable_ptr->slots[index].offset) {
            const HASHTABLE_SLOT_TYPE temp = hashtable_ptr->slots[index];
            hashtable_ptr->slots[index] = current_slot;
            current_slot = temp;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        current_slot.offset++;
    }
    hashtable_ptr->slots[index] = current_slot;
    hashtable_ptr->count++;

    return true;
}

/**
 * @brief Delete a key and it's corresponding value from the hashtable.
 *
 * Asserts hashtable_ptr is not `NULL`.
 *
 * @param[in] hashtable_ptr The hashtable pointer.
 * @param[in] key The key.
 * @return A boolean indicating whether the key was previously contained in the hashtable.
 */
static inline bool JOIN(HASHTABLE_PREFIX, delete)(HASHTABLE_TYPE* hashtable_ptr, const KEY_TYPE key) {
    assert(hashtable_ptr != NULL);

    const size_t key_hash = (size_t)HASH_FUNCTION(key);

    size_t index = key_hash & hashtable_ptr->index_mask;
    size_t max_possible_offset = 0;

    while (hashtable_ptr->slots[index].offset != EMPTY_HASHTABLE_SLOT_OFFSET &&
           max_possible_offset <= hashtable_ptr->slots[index].offset) {

        if (KEY_IS_EQUAL(hashtable_ptr->slots[index].key, key)) {
            // mark as deleted:
            hashtable_ptr->slots[index].offset = EMPTY_HASHTABLE_SLOT_OFFSET;
            hashtable_ptr->count--;

            // reduce offsets as much as possible by moving back offset elements:
            size_t next_index = (index + 1) & hashtable_ptr->index_mask;
            while (hashtable_ptr->slots[next_index].offset != EMPTY_HASHTABLE_SLOT_OFFSET &&
                   hashtable_ptr->slots[next_index].offset > 0) {

                hashtable_ptr->slots[index] = hashtable_ptr->slots[next_index];
                hashtable_ptr->slots[index].offset--;
                hashtable_ptr->slots[next_index].offset = EMPTY_HASHTABLE_SLOT_OFFSET;

                index = next_index;
                next_index = (index + 1) & hashtable_ptr->index_mask;
            }
            return true;
        }
        index = (index + 1) & hashtable_ptr->index_mask;
        max_possible_offset++;
    }
    return false;
}

#undef HASHTABLE_PREFIX
#undef HASHTABLE_TYPE
#undef HASHTABLE_SLOT_TYPE
#undef KEY_TYPE
#undef VALUE_TYPE
#undef KEY_IS_EQUAL
#undef HASH_FUNCTION

#undef PASTE
#undef CONCAT
#undef JOIN
