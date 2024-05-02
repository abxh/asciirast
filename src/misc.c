#include <stdlib.h>

#include "misc.h"

void sleep_portable(size_t millisec) {
#ifdef _WIN32
#include <windows.h>
    Sleep(milliSec);
#elif defined(__linux)
#include <unistd.h>
    usleep(millisec * 1000); // microseconds
#endif
}
