#pragma once

#include <stdlib.h>

#ifdef DEBUG
#define HANDLE_NULL_(func_name) \
    fprintf(stderr, "exiting. function %s at line %d in file %s returned NULL.", func_name, __LINE__, __FILE__);
#else
#define HANDLE_NULL_(func_name) fprintf(stderr, "exiting. memory allocation error in file %s.", __FILE__);
#endif

#define HANDLE_NULL(ptr, func_name) \
    do {                            \
        if (ptr == NULL) {          \
            HANDLE_NULL_(func_name) \
            exit(1);                \
        }                           \
    } while (false)
