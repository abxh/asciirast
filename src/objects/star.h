#pragma once

#include "object.h"

void** star_create(void);

void star_destroy(void** context_ptr);

void star_update(void** context_ptr);

void star_on_key(void** context_ptr, char c);

extern object_ops_type star_ops;
