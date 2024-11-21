#include "hardware/gpio.h"

#include "config.h"

void relays_init() {
    gpio_init(RELAY_HIGHHIGH_PIN);
    gpio_set_dir(RELAY_HIGHHIGH_PIN, GPIO_OUT);
    gpio_put(RELAY_HIGHHIGH_PIN, 1);

    gpio_init(RELAY_FORCEHEAT_PIN);
    gpio_set_dir(RELAY_FORCEHEAT_PIN, GPIO_OUT);
    gpio_put(RELAY_FORCEHEAT_PIN, 1);
}

void set_relay_highhigh(bool on) {
    gpio_put(RELAY_HIGHHIGH_PIN, on ? 0 : 1);
}

void set_relay_forceheat(bool on) {
    gpio_put(RELAY_FORCEHEAT_PIN, on ? 0 : 1);
}
