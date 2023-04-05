#include "core/String.h"
#include "core/Memory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 StringLength(const char* str) {
    return strlen(str);
}

char* StringDuplicate(const char* str) {
    u64 length = StringLength(str);
    char* copy = Allocate(length + 1, MEMORY_TAG_STRING);
    CopyMemory(copy, str, length + 1);
    return copy;
}

b8 StringEqual(const char* str0, const char* str1) {
    return strcmp(str0, str1) == 0;
}

i32 StringFormat(char* dest, const char* format, ...) {
    if (dest) {
        __builtin_va_list argPtr;
        va_start(argPtr, format);
        i32 written = StringFormatV(dest, format, argPtr);
        va_end(argPtr);
        return written;
    }
    return -1;
}

i32 StringFormatV(char* dest, const char* format, void* vaListp) {
    if (dest) {
        // Big, but can fit on the stack.
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, vaListp);
        buffer[written] = 0;
        CopyMemory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}