#ifndef UTILS_H
#define UTILS_H

#include <math.h>

#define UT_ABS(a) ((a) > 0 ? (a) : -(a))
#define UT_PI 3.141592653589
#define UT_TWO_PI 2*UT_PI
#define UT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define UT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define UT_DEG2RAD(deg) ((deg) * UT_PI / 180.0)


/** Linear interpolation (lerp) between two scalars */
static inline float lerp_float(float a, float b, float t) {
    return a + t * (b - a);
}

static inline float lmap_float(float x, float a, float b, float c, float d) {
    float t = (x - a) / (b - a);
    return lerp_float(c, d, t);
}



/* Random number generator - originally by @Skeeto */
enum { XRAND_MAX = 0x7fffffff, LGC_FULL_PERIOD = 0x3243f6a8885a308d};

static unsigned long long xrandom_state = 1;

static void xsrandom(unsigned long long seed) {
    xrandom_state = seed;
}

static int xrandom(void) {
    xrandom_state = xrandom_state*LGC_FULL_PERIOD+ 1;
    return xrandom_state >> 33;
}

#endif // UTILS_H
