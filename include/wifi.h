#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

void wifi_init();
void send_temperature_data(float temperature, bool forcedHeatActive, bool highHighCondition, bool startupCondition, bool emberPreservationActive);

#endif
