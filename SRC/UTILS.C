#include "src/utils.h"

double lerp(double start, double end, double r)
{
    return start * (1.0 - r) + end * r;
}
