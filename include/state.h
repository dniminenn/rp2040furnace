#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "pico/time.h"

typedef struct {
    absolute_time_t forceHeatStart;
    bool highHighCondition;
    bool forcedHeatActive;
    bool startupCondition;
    bool emberPreservationActive;
    bool forceHeatBySwitch;
    bool switchPressed;
} FurnaceState;

#endif // STATE_H
