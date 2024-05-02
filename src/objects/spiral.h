#pragma once

#include "object.h"

void** spiral_create(void);

void spiral_destroy(void** context_ptr);

void spiral_update(void** context_ptr);

extern object_ops_type spiral_ops;
