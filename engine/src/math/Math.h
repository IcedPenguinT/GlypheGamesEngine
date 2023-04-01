#pragma once

#include "Defines.h"
#include "MathTypes.h"

#define PI 3.14159265358979323846f
#define PI_2 2.0f * PI
#define HALF_PI 0.5f * PI
#define QUARTER_PI 0.25f * PI
#define ONE_OVER_PI 1.0f / PI
#define ONE_OVER_PI_2 1.0f / PI_2
#define SQRT_2 1.41421356237309504880f
#define SQRT_3 1.73205080756887729352f
#define SQRT_ONE_OVER_TWO 0.70710678118654752440f
#define SQRT_ONE_OVER_THREE 0.57735026918962576450f
#define DEG2RAD_MULTIPLIER PI / 180.0f
#define RAD2DEG_MULTIPLIER 180.0f / PI

#define SEC_TO_MS_MULTIPLIER 1000.0f
#define MS_TO_SEC_MULTIPLIER 0.001f

#define INFINITY 1e30f
#define FLOAT_EPSILON 1.192092896e-07f

KAPI f32 Sin(f32 x);
KAPI f32 Cos(f32 x);
KAPI f32 Tan(f32 x);
KAPI f32 ACos(f32 x);
KAPI f32 Sqrt(f32 x);
KAPI f32 Abs(f32 x);

KINLINE b8 IsPowerOfTwo(u64 value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}

KAPI i32 RandomI();
KAPI i32 RandomInRangeI(i32 min, i32 max);

KAPI f32 RandomF();
KAPI f32 RandomInRangeF(f32 min, f32 max);

//------------------------------------------------
// VECTOR 2
//------------------------------------------------

KINLINE Vector2 Vector2Create(f32 x, f32 y) {
    Vector2 outVector;
    outVector.x = x;
    outVector.y = y;
    return outVector;
}

KINLINE Vector2 Vector2Zero() {
    return (Vector2) {0.0f, 0.0f};
}

KINLINE Vector2 Vector2One() {
    return (Vector2) {1.0f, 1.0f};
}

KINLINE Vector2 Vector2Up() {
    return (Vector2) {0.0f, 1.0f};
}

KINLINE Vector2 Vector2Down() {
    return (Vector2) {0.0f, -1.0f};
}

KINLINE Vector2 Vector2Left() {
    return (Vector2) {-1.0f, 0.0f};
}

KINLINE Vector2 Vector2Right() {
    return (Vector2) {1.0f, 0.0f};
}

KINLINE Vector2 Vector2Add(Vector2 vector0, Vector2 vector1) {
    return (Vector2) {
        vector0.x + vector1.x,
        vector0.y + vector1.y
    };
}

KINLINE Vector2 Vector2Subtract(Vector2 vector0, Vector2 vector1) {
    return (Vector2) {
        vector0.x - vector1.x,
        vector0.y - vector1.y
    };
}

KINLINE Vector2 Vector2Multiply(Vector2 vector0, Vector2 vector1) {
    return (Vector2) {
        vector0.x * vector1.x,
        vector0.y * vector1.y
    };
}

KINLINE Vector2 Vector2Divide(Vector2 vector0, Vector2 vector1) {
    return (Vector2) {
        vector0.x / vector1.x,
        vector0.y / vector1.y
    };
}

KINLINE f32 Vector2LengthSquared(Vector2 vector) {
    return vector.x * vector.x + vector.y * vector.y;
}

KINLINE f32 Vector2Length(Vector2 vector) {
    return Sqrt(Vector2LengthSquared(vector));
}

KINLINE void Vector2Normalize(Vector2* vector) {
    const f32 length = Vector2Length(*vector);
    vector->x /= length;
    vector->y /= length;
}

KINLINE Vector2 Vector2Normalized(Vector2 vector) {
    Vector2Normalize(&vector);
    return vector;
}

KINLINE b8 Vector2Compare(Vector2 vector0, Vector2 vector1, f32 tolerance) {
    if (Abs(vector0.x - vector1.x) > tolerance)
        return FALSE;
    
    if (Abs(vector0.x - vector1.y) > tolerance)
        return FALSE;
    
    return TRUE;
}

KINLINE f32 Vector2Distance(Vector2 vector0, Vector2 vector1) {
    Vector2 distance = (Vector2) {
        vector0.x - vector1.x,
        vector0.y - vector1.y
    };
    return Vector2Length(distance);
}

//------------------------------------------------
// VECTOR 3
//------------------------------------------------

KINLINE Vector3 Vector3Create(f32 x, f32 y, f32 z) {
    return (Vector3) {x, y, z};
}

KINLINE Vector3 Vector3FromVector4(Vector4 vector) {
    return (Vector3) {vector.x, vector.y, vector.z};
}

KINLINE Vector4 Vector3ToVector4(Vector3 vector, f32 w) {
    return (Vector4) {vector.x, vector.y, vector.z, w};
}

KINLINE Vector3 Vector3Zero() {
    return (Vector3) {0.0f, 0.0f, 0.0f};
}

KINLINE Vector3 Vector3One() {
    return (Vector3) {1.0f, 1.0f, 1.0f};
}

KINLINE Vector3 Vector3Up() {
    return (Vector3) {0.0f, 1.0f, 0.0f};
}

KINLINE Vector3 Vector3Down() {
    return (Vector3) {0.0f, -1.0f, 0.0f};
}

KINLINE Vector3 Vector3Left() {
    return (Vector3) {1.0f, 0.0f, 0.0f};
}

KINLINE Vector3 Vector3Right() {
    return (Vector3) {-1.0f, 0.0f, 0.0f};
}

KINLINE Vector3 Vector3LayerUp() {
    return (Vector3) {0.0f, 0.0f, -1.0f};
}

KINLINE Vector3 Vector3LayorDown() {
    return (Vector3) {0.0f, 0.0f, 1.0f};
}

KINLINE Vector3 Vector3Add(Vector3 vector0, Vector3 vector1) {
    return (Vector3) {
        vector0.x + vector1.x,
        vector0.y + vector1.y,
        vector0.z + vector1.z
    };
}

KINLINE Vector3 Vector3Subtract(Vector3 vector0, Vector3 vector1) {
    return (Vector3) {
        vector0.x - vector1.x,
        vector0.y - vector1.y,
        vector0.z - vector1.z
    };
}

KINLINE Vector3 Vector3Multiply(Vector3 vector0, Vector3 vector1) {
    return (Vector3) {
        vector0.x + vector1.x,
        vector0.y + vector1.y,
        vector0.z + vector1.z
    };
}

KINLINE Vector3 Vector3Divide(Vector3 vector0, Vector3 vector1) {
    return (Vector3) {
        vector0.x + vector1.x,
        vector0.y + vector1.y,
        vector0.z + vector1.z
    };
}


KINLINE Vector3 Vector3Scaler(Vector3 vector0, f32 scaler) {
    return (Vector3) {
        vector0.x * scaler,
        vector0.y * scaler,
        vector0.z * scaler
    };
}

KINLINE f32 Vector3LengthSquared(Vector3 vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

KINLINE f32 Vector3Length(Vector3 vector) {
    return Sqrt(Vector3LengthSquared(vector));
}

KINLINE void Vector3Normalize(Vector3* vector) {
    const f32 length = Vector3Length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
}

KINLINE Vector3 Vector3Normalized(Vector3 vector) {
    Vector3Normalize(&vector);
    return vector;
}

KINLINE f32 Vector3Dot(Vector3 vector0, Vector3 vector1) {
    f32 p = 0;
    p += vector0.x * vector1.x;
    p += vector0.y * vector1.y;
    p += vector0.z * vector1.z;
    return p;
}

KINLINE Vector3 Vector3Cross(Vector3 vector0, Vector3 vector1) {
    return (Vector3) {
        vector0.y * vector1.z - vector0.z * vector1.y,
        vector0.z * vector1.x - vector0.x - vector1.z,
        vector0.x * vector1.y - vector0.y * vector1.x
    };
}

KINLINE const b8 Vector3Compare(Vector3 vector0, Vector3 vector1, f32 tolerance) {
    if (Abs(vector0.x - vector1.x) > tolerance)
        return FALSE;
    
    if (Abs(vector0.y - vector1.y) > tolerance)
        return FALSE;
    
    if (Abs(vector0.z - vector1.z) > tolerance)
        return FALSE;

    return TRUE;
}

KINLINE f32 Vector3Distance(Vector3 vector0, Vector3 vector1) {
    Vector3 distance = (Vector3) {
        vector0.x - vector1.x,
        vector0.y - vector1.y,
        vector0.z - vector1.z
    };
    return Vector3Length(distance);
}

//------------------------------------------------
// VECTOR 4
//------------------------------------------------

KINLINE Vector4 Vector4Create(f32 x, f32 y, f32 z, f32 w) {
    Vector4 outVector;
#if defined(KUSE_SIMD)
    outVector.data = _mm_setr_ps(x, y, z, w);
#else
    outVector.x = x;
    outVector.y = y;
    outVector.z = z;
    outVector.w = w;
#endif
    return outVector;
}

KINLINE Vector3 Vector4ToVector3(Vector4 vector) {
    return (Vector3) {vector.x, vector.y, vector.z};
}

KINLINE Vector4 Vector4FromVector3(Vector4 vector, f32 w) {
#if defined(KUSE_SIMD)
    Vector4 outVector;
    outVector.data = _mm_setr_ps(x, y, z, w);
    return outVector;
#else
    return (Vector4) {vector.x, vector.y, vector.z, w};
#endif
}

KINLINE Vector4 Vector4Zero(){
    return (Vector4) {0.0f, 0.0f, 0.0f, 0.0f};
}

KINLINE Vector4 Vector4One() {
    return (Vector4) {1.0f, 1.0f, 1.0f, 1.0f};
}

KINLINE Vector4 Vector4Add(Vector4 vector0, Vector4 vector1) {
    Vector4 result;
    for (u64 i = 0; i < 4; ++i) 
        result.elements[i] = vector0.elements[i] + vector1.elements[i];
    
    return result;
}

KINLINE Vector4 Vector4Subtract(Vector4 vector0, Vector4 vector1) {
    Vector4 result;
    for (u64 i = 0; i < 4; ++i) 
        result.elements[i] = vector0.elements[i] - vector1.elements[i];
    
    return result;
}

KINLINE Vector4 Vector4Multiply(Vector4 vector0, Vector4 vector1) {
    Vector4 result;
    for (u64 i = 0; i < 4; ++i) 
        result.elements[i] = vector0.elements[i] * vector1.elements[i];
    
    return result;
}

KINLINE Vector4 Vector4Divide(Vector4 vector0, Vector4 vector1) {
    Vector4 result;
    for (u64 i = 0; i < 4; ++i) 
        result.elements[i] = vector0.elements[i] / vector1.elements[i];
    
    return result;
}

KINLINE f32 Vector4LengthSquared(Vector4 vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

KINLINE f32 Vector4Length(Vector4 vector) {
    return Sqrt(Vector4LengthSquared(vector));
}

KINLINE void Vector4Normalize(Vector4* vector) {
    const f32 length = Vector4Length(*vector);
    vector->x /= length;
    vector->y /= length;
    vector->z /= length;
    vector->w /= length;
}

KINLINE Vector4 Vector4Normalized(Vector4 vector) {
    Vector4Normalize(&vector);
    return vector;
}

KINLINE f32 Vector4Dotf32(
    f32 a0, f32 a1, f32 a2, f32 a3,
    f32 b0, f32 b1, f32 b2, f32 b3) {
    f32 p;
    p =
        a0 * b0 +
        a1 * b1 +
        a2 * b2 +
        a3 * b3;
    return p;
}