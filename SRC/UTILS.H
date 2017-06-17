#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DBG(msg, ...) { fprintf(stdout, msg, __VA_ARGS__); fflush(stdout); }
#define LERP(start, end, r) ( start * (1.0 - r) + end * r )
#define MIN(a, b) ( a > b ? b : a )
#define MAX(a, b) ( a > b ? a : b )

#ifdef __cplusplus
}
#endif
#endif
