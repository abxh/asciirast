/**
 * @file stack.h
 * @brief Stack data structure based on a dynamic array.
 */

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @def STACK_PREFIX
 * @brief The stack prefix. Is undefined once header is included.
 * @attention This must be manually defined before including this header file.
 */
#ifndef STACK_PREFIX
#error "must define STACK_PREFIX. defaulting to 'stack'."
#define STACK_PREFIX stack
#endif

/**
 * @def VALUE_TYPE
 * @brief The value type. Is undefined after header is included.
 * @attention This must be manually defined before including this header file.
 */
#ifndef VALUE_TYPE
#error "must define VALUE_TYPE. defaulting to int."
#define VALUE_TYPE int
#endif

/**
 * @def stack_for_each
 * @brief Iterate over the values in the stack from the top value or the first-to-be-popped value.
 * @param[in] stack_ptr Stack pointer.
 * @param[out] count Number of values iterated as `size_t`.
 * @param[out] value Value at `count - 1` as `VALUE_TYPE`.
 */
#define stack_for_each(stack_ptr, count, value) \
    for ((count) = (stack_ptr)->count; (count) > 0 && ((value) = (stack_ptr)->values[(count) - 1], true); (count)--)

/// @cond DO_NOT_DOCUMENT
// helper macros:
#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between
/// @endcond

/* The stack type name. Set to STACK_PREFIX##_##type. */
#ifndef STACK_TYPE
#define STACK_TYPE JOIN(STACK_PREFIX, type)
#endif

/**
 * @brief Generated stack struct type for a given value type.
 */
typedef struct {
    size_t count;       ///< number of values in the stack.
    size_t capacity;    ///< number of values allocated for in the stack.
    VALUE_TYPE* values; ///< pointer to array containing the values in the stack.
} STACK_TYPE;

/**
 * @brief Create a stack with specific initial capacity.
 *
 * @param[in] initial_capacity The initial capacity of the stack.
 * @return Pointer to the created stack.
 * @retval `NULL`
 *   @li If `initial_capacity` is 0.
 *   @li If `initial_capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline STACK_TYPE* JOIN(STACK_PREFIX, create_with_initial_capacity)(const size_t initial_capacity) {
    if (initial_capacity == 0 || initial_capacity > SIZE_MAX / sizeof(VALUE_TYPE)) {
        return NULL;
    }
    STACK_TYPE* stack_ptr = malloc(sizeof(STACK_TYPE));
    if (!stack_ptr) {
        return NULL;
    }
    stack_ptr->values = calloc(initial_capacity, sizeof(VALUE_TYPE));
    if (!stack_ptr->values) {
        free(stack_ptr);
        return NULL;
    }
    stack_ptr->count = 0;
    stack_ptr->capacity = initial_capacity;

    return stack_ptr;
}

/**
 * @brief Create a stack with default capacity (512).
 *
 * @return The stack pointer.
 */
static inline STACK_TYPE* JOIN(STACK_PREFIX, create)(void) {
    return JOIN(STACK_PREFIX, create_with_initial_capacity)(512);
}

/**
 * @brief Destroy a stack and free the used memory.
 *
 * @warning May not be called twice in a row on the same stack.
 */
static inline void JOIN(STACK_PREFIX, destroy)(STACK_TYPE* stack_ptr) {
    if (!stack_ptr) {
        return;
    }
    free(stack_ptr->values);
    free(stack_ptr);
}

/**
 * @brief Clone an existing stack.
 *
 * @param stack_ptr The pointer of the stack to clone.
 * @return A pointer to the clone of the original stack.
 * @retval `NULL`
 *   @li If no memory space is available.
 *   @li If stack_ptr is `NULL`.
 */
static inline STACK_TYPE* JOIN(STACK_PREFIX, clone)(const STACK_TYPE* stack_ptr) {
    if (!stack_ptr) {
        return NULL;
    }

    STACK_TYPE* other_stack_ptr = JOIN(STACK_PREFIX, create_with_initial_capacity)(stack_ptr->capacity);
    if (!other_stack_ptr) {
        return NULL;
    }

    for (size_t i = 0; i < stack_ptr->count; i++) {
        other_stack_ptr->values[i] = stack_ptr->values[i];
    }

    other_stack_ptr->count = stack_ptr->count;
    other_stack_ptr->capacity = stack_ptr->capacity;

    return other_stack_ptr;
}

/**
 * @brief Get the number of values in the stack.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return The number of values as `size_t`.
 */
static inline size_t JOIN(STACK_PREFIX, count)(const STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);

    return stack_ptr->count;
}

/**
 * @brief Get the number of values allocated for in the stack. Capacity is grown as needed.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return The current capacity as `size_t`.
 */
static inline size_t JOIN(STACK_PREFIX, capacity)(const STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);

    return stack_ptr->capacity;
}

/**
 * @brief Check if stack is empty.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return A boolean indicating whether the stack is empty.b
 */
static inline bool JOIN(STACK_PREFIX, is_empty)(const STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);

    return 0 == stack_ptr->count;
}

/**
 * @brief Get value at index starting from the bottom of the stack as 0.
 *
 * Asserts stack_ptr is not `NULL` and index is strictly less than stack count.
 *
 * @param[in] stack_ptr The stack pointer.
 * @param[in] index Index at which the value lies.
 * @return The value as VALUE_TYPE.
 */
static inline VALUE_TYPE JOIN(STACK_PREFIX, at)(const STACK_TYPE* stack_ptr, const size_t index) {
    assert(NULL != stack_ptr);
    assert(index < stack_ptr->count);

    return stack_ptr->values[index];
}

/**
 * @brief Get the value from the top of the stack.
 *
 * Asserts stack_ptr is not `NULL` and stack is not empty.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return The value as VALUE_TYPE.
 */
static inline VALUE_TYPE JOIN(STACK_PREFIX, top)(const STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);
    assert(!JOIN(STACK_PREFIX, is_empty)(stack_ptr));

    return stack_ptr->values[stack_ptr->count - 1];
}

/**
 * @brief Peek at the stack and get it's first-to-be-popped top value.
 *
 * Asserts stack_ptr is not `NULL` and stack is not empty.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return The value as VALUE_TYPE.
 */
static inline VALUE_TYPE JOIN(STACK_PREFIX, peek)(const STACK_TYPE* stack_ptr) {
    return JOIN(STACK_PREFIX, top)(stack_ptr);
}

/**
 * @brief Resize and grow the stack array manually.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 * @param[in] new_capacity The new capacity.
 * @return A boolean indicating whether the stack was able to be resized to grow.
 * @retval false
 *   @li If `new_capacity` is smaller than or equal to the current capacity.
 *   @li If `new_capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If no memory space is available.
 */
static inline bool JOIN(STACK_PREFIX, grow)(STACK_TYPE* stack_ptr, const size_t new_capacity) {
    assert(NULL != stack_ptr);

    if (new_capacity <= stack_ptr->capacity || new_capacity > SIZE_MAX / sizeof(VALUE_TYPE)) {
        return false;
    }
    void* temp_ptr = realloc(stack_ptr->values, new_capacity * sizeof(VALUE_TYPE));
    if (temp_ptr == NULL) {
        return false;
    }
    stack_ptr->values = (VALUE_TYPE*)temp_ptr;
    stack_ptr->capacity = new_capacity;

    return true;
}

/**
 * @brief Push a value onto the stack.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 g @param[in] value The value.
 * @return A boolean indicating whether the value was stored, depending on whether
 *         the stack could be resized if it becomes full once the value is stored.
 * @retval false
 *   @li If stack is resized and `2 * current capacity * sizeof(VALUE_TYPE)` cannot be expressed with `size_t`.
 *   @li If stack is resized and no memory space is available.
 */
static inline bool JOIN(STACK_PREFIX, push)(STACK_TYPE* stack_ptr, const VALUE_TYPE value) {
    assert(NULL != stack_ptr);

    if (stack_ptr->count + 1 == stack_ptr->capacity && !JOIN(STACK_PREFIX, grow)(stack_ptr, 2 * stack_ptr->capacity)) {
        return false;
    }
    stack_ptr->values[stack_ptr->count++] = value;

    return true;
}

/**
 * @brief Pop a value from the stack and return the value.
 *
 * Asserts stack_ptr is not `NULL` and stack is not empty.
 *
 * @param[in] stack_ptr The stack pointer.
 * @return The value as `VALUE_TYPE`.
 */
static inline VALUE_TYPE JOIN(STACK_PREFIX, pop)(STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);
    assert(!JOIN(STACK_PREFIX, is_empty)(stack_ptr));

    return stack_ptr->values[--stack_ptr->count];
}

/**
 * @brief Clear the stack.
 *
 * Asserts stack_ptr is not `NULL`.
 *
 * @param[in] stack_ptr The stack pointer.
 */
static inline void JOIN(STACK_PREFIX, clear)(STACK_TYPE* stack_ptr) {
    assert(NULL != stack_ptr);

    stack_ptr->count = 0;
}

#undef STACK_PREFIX
#undef VALUE_TYPE
#undef STACK_TYPE

#undef PASTE
#undef CONCAT
#undef JOIN

// vim: ft=c
