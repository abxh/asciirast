#pragma once

typedef char* cmd_desc;
#define P
#define T cmd_desc
#include <set.h>

#include <string.h>

static inline int cmd_desc_cmp(cmd_desc* a, cmd_desc* b) {
    return strcmp(*a, *b);
}
