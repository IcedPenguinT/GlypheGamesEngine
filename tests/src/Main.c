#include "TestManager.h"

#include "memory/LinearAllocatorTests.h"

#include <core/Logger.h>

int main() {
    // Always initalize the test manager first.
    TestManagerInit();

    // TODO: add test registrations here.
    LinearAllocatorRegisterTests();


    KDEBUG("Starting tests...");

    // Execute tests
    TestManagerRunTests();

    return 0;
}