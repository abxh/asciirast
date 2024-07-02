#pragma once

typedef struct {
    char value[20];
} key_comb_type;

typedef struct {
    char value[20];
} command_name_type;

#define HASHTABLE_PREFIX cmdht
#define KEY_TYPE key_comb_type
#define VALUE_TYPE command_name_type
#define KEY_IS_EQUAL(a, b) (strncmp(a.value, b.value, sizeof(key_comb_type)) == 0)
#include "data_structures/hashtable.h"
