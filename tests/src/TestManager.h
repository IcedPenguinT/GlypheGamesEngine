#pragma once

#include <Defines.h>

#define BYPASS 2

typedef u8 (*PFNTest)();

void TestManagerInit();

void TestManagerRegisterTest(PFNTest, char* desc);

void TestManagerRunTests();