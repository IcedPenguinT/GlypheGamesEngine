#pragma once

#include "Defines.h"

KAPI u64 StringLength(const char* str);

KAPI char* StringDuplicate(const char* str);

KAPI b8 StringEqual(const char* str0, const char* str1);

KAPI i32 StringFormat(char* dest, const char* format, ...);

KAPI i32 StringFormatV(char* dest, const char* format, void* va_list);