
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux
#include <unistd.h>
#endif

#include "misc.h"

void sleep_portable(size_t millisec) {
#ifdef _WIN32
    Sleep(milliSec);
#endif
#ifdef __linux
    usleep(millisec * 1000);
#endif
}
