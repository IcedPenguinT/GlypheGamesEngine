#pragma once

#include "Defines.h"
#include "MathTypes.h"

#include "core/Memory.h"

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
        return false;
    
    if (Abs(vector0.x - vector1.y) > tolerance)
        return false;
    
    return true;
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
        return false;
    
    if (Abs(vector0.y - vector1.y) > tolerance)
        return false;
    
    if (Abs(vector0.z - vector1.z) > tolerance)
        return false;

    return true;
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
    f32 dot;
    dot =
        a0 * b0 +
        a1 * b1 +
        a2 * b2 +
        a3 * b3;
    return dot;
}

KINLINE Matrix4 Matrix4Identity() {
    Matrix4 outMatrix;
    ZeroMemory(outMatrix.data, sizeof(f32) * 16);
    outMatrix.data[0] = 1.0f;
    outMatrix.data[5] = 1.0f;
    outMatrix.data[10] = 1.0f;
    outMatrix.data[15] = 1.0f;
    return outMatrix;
}

KINLINE Matrix4 Matrix4Multiply(Matrix4 matrix0, Matrix4 matrix1) {
    Matrix4 outMatrix = Matrix4Identity();

    const f32* m1Ptr = matrix0.data;
    const f32* m2Ptr = matrix1.data;
    f32* dstPtr = outMatrix.data;

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            *dstPtr =
                m1Ptr[0] * m2Ptr[0 + j] +
                m1Ptr[1] * m2Ptr[4 + j] +
                m1Ptr[2] * m2Ptr[8 + j] +
                m1Ptr[3] * m2Ptr[12 + j];
            dstPtr++;
        }
        m1Ptr += 4;
    }
    return outMatrix;
}

KINLINE Matrix4 Matrix4Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearClip, f32 farClip) {
    Matrix4 outMatrix = Matrix4Identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (nearClip - farClip);

    outMatrix.data[0] = -2.0f * lr;
    outMatrix.data[5] = -2.0f * bt;
    outMatrix.data[10] = 2.0f * nf;

    outMatrix.data[12] = (left + right) * lr;
    outMatrix.data[13] = (top + bottom) * bt;
    outMatrix.data[14] = (farClip + nearClip) * nf;
    return outMatrix;
}

KINLINE Matrix4 Matrix4Perspective(f32 fovRadians, f32 aspectRatio, f32 nearClip, f32 farClip) {
    f32 halfTanFov = Tan(fovRadians * 0.5f);
    Matrix4 outMatrix;
    ZeroMemory(outMatrix.data, sizeof(f32) * 16);
    outMatrix.data[0] = 1.0f / (aspectRatio * halfTanFov);
    outMatrix.data[5] = 1.0f / halfTanFov;
    outMatrix.data[10] = -((farClip + nearClip) / (farClip - nearClip));
    outMatrix.data[11] = -1.0f;
    outMatrix.data[14] = -((2.0f * farClip * nearClip) / (farClip - nearClip));
    return outMatrix;
}

KINLINE Matrix4 Matrix4LookAt(Vector3 position, Vector3 target, Vector3 up) {
    Matrix4 outMatrix;
    Vector3 zAxis;
    zAxis.x = target.x - position.x;
    zAxis.y = target.y - position.y;
    zAxis.z = target.z - position.z;

    zAxis = Vector3Normalized(zAxis);
    Vector3 xAxis = Vector3Normalized(Vector3Cross(zAxis, up));
    Vector3 yAxis = Vector3Cross(xAxis, zAxis);

    outMatrix.data[0] = xAxis.x;
    outMatrix.data[1] = yAxis.x;
    outMatrix.data[2] = -zAxis.x;
    outMatrix.data[3] = 0;
    outMatrix.data[4] = xAxis.y;
    outMatrix.data[5] = yAxis.y;
    outMatrix.data[6] = -zAxis.y;
    outMatrix.data[7] = 0;
    outMatrix.data[8] = xAxis.z;
    outMatrix.data[9] = yAxis.z;
    outMatrix.data[10] = -zAxis.z;
    outMatrix.data[11] = 0;
    outMatrix.data[12] = -Vector3Dot(xAxis, position);
    outMatrix.data[13] = -Vector3Dot(yAxis, position);
    outMatrix.data[14] = Vector3Dot(zAxis, position);
    outMatrix.data[15] = 1.0f;

    return outMatrix;
}

KINLINE Matrix4 Matrix4Transposed(Matrix4 matrix) {
    Matrix4 outMatrix = Matrix4Identity();
    outMatrix.data[0] = matrix.data[0];
    outMatrix.data[1] = matrix.data[4];
    outMatrix.data[2] = matrix.data[8];
    outMatrix.data[3] = matrix.data[12];
    outMatrix.data[4] = matrix.data[1];
    outMatrix.data[5] = matrix.data[5];
    outMatrix.data[6] = matrix.data[9];
    outMatrix.data[7] = matrix.data[13];
    outMatrix.data[8] = matrix.data[2];
    outMatrix.data[9] = matrix.data[6];
    outMatrix.data[10] = matrix.data[10];
    outMatrix.data[11] = matrix.data[14];
    outMatrix.data[12] = matrix.data[3];
    outMatrix.data[13] = matrix.data[7];
    outMatrix.data[14] = matrix.data[11];
    outMatrix.data[15] = matrix.data[15];
    return outMatrix;
}

KINLINE Matrix4 Matrix4Inverse(Matrix4 matrix) {
    const f32* m = matrix.data;

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    Matrix4 outMatrix;
    f32* o = outMatrix.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return outMatrix;
}

KINLINE Matrix4 Matrix4Translation(Vector3 position) {
    Matrix4 outMatrix = Matrix4Identity();
    outMatrix.data[12] = position.x;
    outMatrix.data[13] = position.y;
    outMatrix.data[14] = position.z;
    return outMatrix;
}

/**
 * @brief Returns a scale matrix using the provided scale.
 * 
 * @param scale The 3-component scale.
 * @return A scale matrix.
 */
KINLINE Matrix4 Matrix4Scale(Vector3 scale) {
    Matrix4 outMatrix = Matrix4Identity();
    outMatrix.data[0] = scale.x;
    outMatrix.data[5] = scale.y;
    outMatrix.data[10] = scale.z;
    return outMatrix;
}

KINLINE Matrix4 Matrix4EulerX(f32 angleRadians) {
    Matrix4 outMatrix = Matrix4Identity();
    f32 c = Cos(angleRadians);
    f32 s = Sin(angleRadians);

    outMatrix.data[5] = c;
    outMatrix.data[6] = s;
    outMatrix.data[9] = -s;
    outMatrix.data[10] = c;
    return outMatrix;
}
KINLINE Matrix4 Matrix4EulerY(f32 angleRadians) {
    Matrix4 outMatrix = Matrix4Identity();
    f32 c = Cos(angleRadians);
    f32 s = Sin(angleRadians);

    outMatrix.data[0] = c;
    outMatrix.data[2] = -s;
    outMatrix.data[8] = s;
    outMatrix.data[10] = c;
    return outMatrix;
}
KINLINE Matrix4 Matrix4EulerZ(f32 angleRadians) {
    Matrix4 outMatrix = Matrix4Identity();

    f32 c = Cos(angleRadians);
    f32 s = Sin(angleRadians);

    outMatrix.data[0] = c;
    outMatrix.data[1] = s;
    outMatrix.data[4] = -s;
    outMatrix.data[5] = c;
    return outMatrix;
}

KINLINE Matrix4 Matrix4EulerXyz(f32 xRadians, f32 yRadians, f32 zRadians) {
    Matrix4 rx = Matrix4EulerX(xRadians);
    Matrix4 ry = Matrix4EulerY(yRadians);
    Matrix4 rz = Matrix4EulerZ(zRadians);
    Matrix4 outMatrix = Matrix4Multiply(rx, ry);
    outMatrix = Matrix4Multiply(outMatrix, rz);
    return outMatrix;
}

KINLINE Vector3 Matrix4Forward(Matrix4 matrix) {
    Vector3 forward;
    forward.x = -matrix.data[2];
    forward.y = -matrix.data[6];
    forward.z = -matrix.data[10];
    Vector3Normalize(&forward);
    return forward;
}

/**
 * @brief Returns a backward vector relative to the provided matrix.
 * 
 * @param matrix The matrix from which to base the vector.
 * @return A 3-component directional vector.
 */
KINLINE Vector3 Matrix4_backward(Matrix4 matrix) {
    Vector3 backward;
    backward.x = matrix.data[2];
    backward.y = matrix.data[6];
    backward.z = matrix.data[10];
    Vector3Normalize(&backward);
    return backward;
}

KINLINE Vector3 Matrix4Up(Matrix4 matrix) {
    Vector3 up;
    up.x = matrix.data[1];
    up.y = matrix.data[5];
    up.z = matrix.data[9];
    Vector3Normalize(&up);
    return up;
}

KINLINE Vector3 Matrix4Down(Matrix4 matrix) {
    Vector3 down;
    down.x = -matrix.data[1];
    down.y = -matrix.data[5];
    down.z = -matrix.data[9];
    Vector3Normalize(&down);
    return down;
}

KINLINE Vector3 Matrix4Left(Matrix4 matrix) {
    Vector3 right;
    right.x = -matrix.data[0];
    right.y = -matrix.data[4];
    right.z = -matrix.data[8];
    Vector3Normalize(&right);
    return right;
}

KINLINE Vector3 Matrix4Right(Matrix4 matrix) {
    Vector3 left;
    left.x = matrix.data[0];
    left.y = matrix.data[4];
    left.z = matrix.data[8];
    Vector3Normalize(&left);
    return left;
}

// ------------------------------------------
// Quaternion
// ------------------------------------------

KINLINE Quaternion QuaternionIdentity() {
    return (Quaternion){0, 0, 0, 1.0f};
}

KINLINE f32 QuaternionNormal(Quaternion q) {
    return Sqrt(
        q.x * q.x +
        q.y * q.y +
        q.z * q.z +
        q.w * q.w);
}

KINLINE Quaternion QuaternionNormalize(Quaternion q) {
    f32 normal = QuaternionNormal(q);
    return (Quaternion){
        q.x / normal,
        q.y / normal,
        q.z / normal,
        q.w / normal};
}

KINLINE Quaternion QuaternionConjugate(Quaternion q) {
    return (Quaternion){
        -q.x,
        -q.y,
        -q.z,
        q.w};
}

KINLINE Quaternion QuaternionInverse(Quaternion q) {
    return QuaternionNormalize(QuaternionConjugate(q));
}

KINLINE Quaternion QuaternionMul(Quaternion quaternion0, Quaternion quaternion1) {
    Quaternion outQuaternionernion;

    outQuaternionernion.x = quaternion0.x * quaternion1.w +
                       quaternion0.y * quaternion1.z -
                       quaternion0.z * quaternion1.y +
                       quaternion0.w * quaternion1.x;

    outQuaternionernion.y = -quaternion0.x * quaternion1.z +
                       quaternion0.y * quaternion1.w +
                       quaternion0.z * quaternion1.x +
                       quaternion0.w * quaternion1.y;

    outQuaternionernion.z = quaternion0.x * quaternion1.y -
                       quaternion0.y * quaternion1.x +
                       quaternion0.z * quaternion1.w +
                       quaternion0.w * quaternion1.z;

    outQuaternionernion.w = -quaternion0.x * quaternion1.x -
                       quaternion0.y * quaternion1.y -
                       quaternion0.z * quaternion1.z +
                       quaternion0.w * quaternion1.w;

    return outQuaternionernion;
}

KINLINE f32 QuaternionDot(Quaternion quaternion0, Quaternion quaternion1) {
    return quaternion0.x * quaternion1.x +
           quaternion0.y * quaternion1.y +
           quaternion0.z * quaternion1.z +
           quaternion0.w * quaternion1.w;
}

KINLINE Matrix4 QuaternionToMatrix4(Quaternion q) {
    Matrix4 outMatrix = Matrix4Identity();

    // https://stackoverflow.com/questions/1556260/convert-Quaternionernion-rotation-to-rotation-matrix

    Quaternion n = QuaternionNormalize(q);

    outMatrix.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    outMatrix.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    outMatrix.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    outMatrix.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    outMatrix.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    outMatrix.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    outMatrix.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    outMatrix.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    outMatrix.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return outMatrix;
}

// Calculates a rotation matrix based on the Quaternionernion and the passed in center point.
KINLINE Matrix4 QuaternionToRotationMatrix(Quaternion q, Vector3 center) {
    Matrix4 outMatrix;

    f32* o = outMatrix.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return outMatrix;
}

KINLINE Quaternion QuaternionFromAxisAngle(Vector3 axis, f32 angle, b8 normalize) {
    const f32 halfAngle = 0.5f * angle;
    f32 s = Sin(halfAngle);
    f32 c = Cos(halfAngle);

    Quaternion q = (Quaternion){s * axis.x, s * axis.y, s * axis.z, c};
    if (normalize) {
        return QuaternionNormalize(q);
    }
    return q;
}

KINLINE Quaternion QuaternionSlerp(Quaternion quaternion0, Quaternion quaternion1, f32 percentage) {
    Quaternion outQuaternionernion;
    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit Quaternionernions are valid rotations.
    // Normalize to avoid undefined behavior.
    Quaternion v0 = QuaternionNormalize(quaternion0);
    Quaternion v1 = QuaternionNormalize(quaternion1);

    // Compute the cosine of the angle between the two vectors.
    f32 dot = QuaternionDot(v0, v1);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that v1 and -v1 are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one Quaternionernion.
    if (dot < 0.0f) {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const f32 DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        outQuaternionernion = (Quaternion){
            v0.x + ((v1.x - v0.x) * percentage),
            v0.y + ((v1.y - v0.y) * percentage),
            v0.z + ((v1.z - v0.z) * percentage),
            v0.w + ((v1.w - v0.w) * percentage)};

        return QuaternionNormalize(outQuaternionernion);
    }

    // Since dot is in range [0, DOT_THRESHOLD], acos is safe
    f32 theta0 = ACos(dot);          // theta0 = angle between input vectors
    f32 theta = theta0 * percentage;  // theta = angle between v0 and result
    f32 sinTheta = Sin(theta);       // compute this value only once
    f32 sinTheta0 = Sin(theta0);   // compute this value only once

    f32 s0 = Cos(theta) - dot * sinTheta / sinTheta0;  // == sin(theta0 - theta) / sin(theta0)
    f32 s1 = sinTheta / sinTheta0;

    return (Quaternion){
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)};
}

KINLINE f32 DegToRad(f32 degrees) {
    return degrees * DEG2RAD_MULTIPLIER;
}

KINLINE f32 RadToDeg(f32 radians) {
    return radians * RAD2DEG_MULTIPLIER;
}