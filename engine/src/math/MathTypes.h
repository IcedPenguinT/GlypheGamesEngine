#pragma once

#include "Defines.h"

typedef union Vector2Union {
    f32 elements[2];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
    };
} Vector2;

typedef union Vector3Union {
    f32 elements[3];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
        union {
            f32 z, b, p, w;
        };
    };
} Vector3;

typedef union Vector4Union {
    #if defined(KUSE_SIMD)
        alignas(16) __m128 data;
    #endif
        alignas(16) f32 elements[4];
    union {
        struct {
            union {
                f32 x, r, s;
            };
            union {
                f32 y, g, t;
            };
            union {
                f32 z, b, p;
            };
            union {
                f32 w, a, q;
            };
        };
    };
} Vector4;

typedef Vector4 quaternion;