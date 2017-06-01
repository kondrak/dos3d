#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define DBG(msg, ...) { fprintf(stdout, msg, __VA_ARGS__); fflush(stdout); }
#define LERP(start, end, r) ( start * (1.0 - r) + end * r )

#endif
