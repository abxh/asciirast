
#ifdef WIN32
#include <windows.h>
#else
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <unistd.h>
#endif

#include "misc.h"

void sleep_ms(int milliseconds) {
    // cross-platform sleep function
    // taken from and modified: https://stackoverflow.com/a/28827188

#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}
