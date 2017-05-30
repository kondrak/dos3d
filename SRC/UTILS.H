#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define DBG(msg, ...) { fprintf(stdout, msg, __VA_ARGS__); fflush(stdout); }

double lerp(double start, double end, double r);

#endif
