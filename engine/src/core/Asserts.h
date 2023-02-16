#pragma once

#include "Defines.h"
#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

KAPI void ReportAssertionFailed(const char* expression, const char* message, const char* file, i32 line);

#define KASSERT(expr)                                                \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            ReportAssertionFailed(#expr, "", __FILE__, __LINE__);    \
            debugBreak();                                            \
        }                                                            \
    }

#define KASSERTMSG(expr, message)                                         \
    {                                                                     \
        if (expr) {                                                       \
        } else {                                                          \
            ReportAssertionFailed(#expr, message, __FILE__, __LINE__);    \
            debugBreak();                                                 \
        }                                                                 \
    }

#ifdef _DEBUG
#define KASSERTDEBUG(expr)                                           \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            ReportAssertionFailed(#expr, "", __FILE__, __LINE__);    \
            debugBreak();                                            \
        }                                                            \
    }
#else
#define KASSERTDEBUG(expr)  // Does nothing at all
#endif

#else
#define KASSERT(expr)               // Does nothing at all
#define KASSERTMSG(expr, message)  // Does nothing at all
#define KASSERTDEBUG(expr)         // Does nothing at all
#endif