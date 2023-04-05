#include "TestManager.h"

#include <containers/Darray.h>
#include <core/Logger.h>
#include <core/String.h>
#include <core/Clock.h>

typedef struct TestEntry {
    PFNTest func;
    char* desc;
} TestEntry;

static TestEntry* tests;

void TestManagerInit() {
    tests = DarrayCreate(TestEntry);
}

void TestManagerRegisterTest(u8 (*PFNTest)(), char* desc) {
    TestEntry e;
    e.func = PFNTest;
    e.desc = desc;
    DarrayPush(tests, e);
}

void TestManagerRunTests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = DarrayLength(tests);

    Clock totalTime;
    ClockStart(&totalTime);

    for (u32 i = 0; i < count; ++i) {
        Clock testTime;
        ClockStart(&testTime);
        u8 result = tests[i].func();
        ClockUpdate(&testTime);

        if (result == true) {
            ++passed;
        } else if (result == BYPASS) {
            KWARNING("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        } else {
            KERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        StringFormat(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        ClockUpdate(&totalTime);
        KINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, testTime.elapsed, totalTime.elapsed);
    }

    ClockStop(&totalTime);

    KINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}