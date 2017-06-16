#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DBG(msg, ...) { fprintf(stdout, msg, __VA_ARGS__); fflush(stdout); }
#define LERP(start, end, r) ( start * (1.0 - r) + end * r )

#ifdef __cplusplus
}
#endif
#endif
