#pragma once

#include "Defines.h"

enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

KAPI void* _darrayCreate(u64 length, u64 stride);
KAPI void _darrayDestroy(void* array);

KAPI u64 _darrayFieldGet(void* array, u64 field);
KAPI void _darrayFieldSet(void* array, u64 field, u64 value);

KAPI void* _darrayResize(void* array);

KAPI void* _darrayPush(void* array, const void* valuePtr);
KAPI void _darrayPop(void* array, void* dest);

KAPI void* _darrayPopAt(void* array, u64 index, void* dest);
KAPI void* _darrayInsertAt(void* array, u64 index, void* valuePtr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define DarrayCreate(type)              \
    _darrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define DarrayReserve(type, capacity)   \
    _darrayCreate(capacity, sizeof(type))

#define DarrayDestroy(array) _darrayDestroy(array);

#define DarrayPush(array, value)        \
{                                       \
    typeof(value) temp = value;         \
    array = _darrayPush(array, &temp);  \
}                                       \

#define DarrayPop(array, valuePtr)      \
    _darrayPop(array, valuePtr)

#define DarrayInsertAt(array, index, value)         \
{                                                   \
    typeof(value) temp = value;                     \
    array = _darrayInsertAt(array, index, &temp);   \
}                                                   \

#define DarrayPopAt(array, index, valuePtr) \
    _darrayPopAt(array, index, valuePtr)

#define DarrayClear(array)  \
    _darrayFieldSet(array, DARRAY_LENGTH, 0)

#define DarrayCapacity(array) \
    _darrayFieldGet(array, DARRAY_CAPACITY)

#define DarrayLength(array) \
    _darrayFieldGet(array, DARRAY_LENGTH)

#define DarrayStride(array) \
    _darrayFieldGet(array, DARRAY_STRIDE)

#define DarrayLengthSet(array, value) \
    _darrayFieldSet(array, DARRAY_LENGTH, value)