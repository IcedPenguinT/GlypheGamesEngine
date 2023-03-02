#include "containers/Darray.h"
#include "core/Memory.h"
#include "core/Logger.h"

void* _darrayCreate(u64 length, u64 stride) {
    u64 headerSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 arraySize = length * stride;
    u64* newArray = Allocate(headerSize + arraySize, MEMORY_TAG_DARRAY);
    SetMemory(newArray, 0, headerSize + arraySize);
    newArray[DARRAY_CAPACITY] = length;
    newArray[DARRAY_LENGTH] = 0;
    newArray[DARRAY_STRIDE] = stride;
    return (void*)(newArray + DARRAY_FIELD_LENGTH);

}

void _darrayDestroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 headerSize = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 totalSize = headerSize + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    Free(header, totalSize, MEMORY_TAG_DARRAY);
}

u64 _darrayFieldGet(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darrayFieldSet(void* array, u64 field, u64 value){
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darrayResize(void* array) {
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    void* temp = _darrayCreate((DARRAY_RESIZE_FACTOR * DarrayCapacity(array)), stride);
    CopyMemory(temp, array, length * stride);
    _darrayFieldSet(temp, DARRAY_LENGTH, length);
    _darrayDestroy(array);
    return temp;
}

void* _darrayPush(void* array, const void* valuePtr) {
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if (length >= DarrayCapacity(array))
        array = _darrayResize(array);
    
    u64 addr = (u64)array;
    addr += (length * stride);
    CopyMemory((void*)addr, valuePtr, stride);
    _darrayFieldSet(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darrayPop(void* array, void* dest) {
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    CopyMemory(dest, (void*)addr, stride);
    _darrayFieldSet(array, DARRAY_LENGTH, length -1);
}

void* _darrayPopAt(void* array, u64 index, void* dest) {
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if (index >= length) {
        KERROR("Index outside of the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }

    u64 addr = (u64)array;
    CopyMemory(dest, (void*)(addr + (index * stride)), stride);

    if (index != length - 1){
        CopyMemory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index)
        );
    }
    _darrayFieldSet(array, DARRAY_LENGTH, length -1);
    return array;
}

void* _darrayInsertAt(void* array, u64 index, void* valuePtr) {
    u64 length = DarrayLength(array);
    u64 stride = DarrayStride(array);
    if (index >= length) {
        KERROR("Index outside of the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }

    if(length >= DarrayCapacity(array))
        array = _darrayResize(array);

    u64 addr = (u64)array;

    if (index != length - 1){
        CopyMemory(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index)
        );
    }

    CopyMemory((void*)(addr + (index * stride)), valuePtr, stride);

    _darrayFieldSet(array, DARRAY_LENGTH, length + 1);
    return array;
}