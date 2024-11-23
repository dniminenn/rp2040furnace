#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

#include "state.h"

bool wifi_check_connectivity();
void wifi_init();
void wifi_connect();
void send_temperature_data(float temperature, const FurnaceState *state);

#endif
