#pragma once
#include "Defines.h"

typedef struct Clock {
    f64 startTime;
    f64 elapsed;
} Clock;

KAPI void ClockUpdate(Clock* clock);
KAPI void ClockStart(Clock* clock);
KAPI void ClockStop(Clock* clock);