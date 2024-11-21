#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>

void relays_init();
void set_relay_highhigh(bool on);
void set_relay_forceheat(bool on);

#endif
