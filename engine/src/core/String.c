#include "core/String.h"
#include "core/Memory.h"

#include <string.h>

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