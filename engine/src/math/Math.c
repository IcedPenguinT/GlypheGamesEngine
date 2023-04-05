#include "Math.h"
#include "platform/Platform.h"

#include <math.h>
#include <stdlib.h>

static b8 randSeeded = false;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 Sin(f32 x) {
    return sinf(x);
}

f32 Cos(f32 x) {
    return cosf(x);
}

f32 Tan(f32 x) {
    return tanf(x);
}

f32 ACos(f32 x) {
    return acosf(x);
}

f32 Sqrt(f32 x) {
    return sqrtf(x);
}

f32 Abs(f32 x) {
    return fabsf(x);
}

i32 RandomI() {
    if (!randSeeded) {
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return rand();
}

i32 RandomInRangeI(i32 min, i32 max) {
    if (!randSeeded) {
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 RandomF() {
    return (float)RandomI() / (f32)RAND_MAX;
}

f32 RandomInRangeF(f32 min, f32 max) {
    return min + ((float)RandomI() / ((f32)RAND_MAX / (max - min)));
}