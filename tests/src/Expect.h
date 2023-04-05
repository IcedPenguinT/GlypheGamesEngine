#include <core/Logger.h>
#include <math/Math.h>

#define ExpectShouldBe(expected, actual)                                                                \
    if (actual != expected) {                                                                           \
        KERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                   \
    }

#define ExpectShouldNotBe(expected, actual)                                                                      \
    if (actual == expected) {                                                                                    \
        KERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                            \
    }

#define ExpectFloatToBe(expected, actual)                                                           \
    if (kabs(expected - actual) > 0.001f) {                                                         \
        KERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                               \
    }

#define ExpectToBeTrue(actual)                                                         \
    if (actual != true) {                                                              \
        KERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
    }

#define ExpectToBeFalse(actual)                                                     \
    if (actual != false) {                                                             \
        KERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
}