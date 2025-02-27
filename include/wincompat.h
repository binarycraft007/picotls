#ifndef WINCOMPAT_H
#define WINCOMPAT_H

#include <stdint.h>
#define ssize_t int
#include <winsock2.h>
#include <ws2tcpip.h>
#include <malloc.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#ifndef gettimeofday
#define gettimeofday wintimeofday

#ifndef __attribute__
#define __attribute__(X)
#endif

#ifdef __cplusplus
extern "C" {
#endif
struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of DST correction */
};

int wintimeofday(struct timeval *tv, struct timezone *tz);

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#endif /* _MSC_VER */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

#endif /* WINCOMPAT_H */
