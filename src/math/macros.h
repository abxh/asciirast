#pragma once

// misc:
#ifndef PASTE

#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between

#endif
