#pragma once
#include "Defines.h"

typedef struct Clock {
    f64 startTime;
    f64 elapsed;
} Clock;

void ClockUpdate(Clock* clock);
void ClockStart(Clock* clock);
void ClockStop(Clock* clock);