#include <stdio.h>
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "config.h"
#include "max6675.h"
#include "relays.h"
#include "wifi.h"

static absolute_time_t forceHeatStart;
static volatile bool highHighCondition = false;
static volatile bool forcedHeatActive = false;
static volatile bool startupCondition = false;
static volatile bool emberPreservationActive = false;
static volatile bool forceHeatBySwitch = false;
static volatile bool switchPressed = false;

void monitor_door_switch() {
    absolute_time_t pressStart;
    bool isPressing = false;
    while (true) {
        bool switchState = gpio_get(DOOR_SWITCH_PIN);
        if (switchState) {
            if (!isPressing) {
                isPressing = true;
                pressStart = get_absolute_time();
            } else {
                int64_t pressDuration = absolute_time_diff_us(pressStart, get_absolute_time());
                if (pressDuration >= DOOR_OPEN_TIME_MS * 1000 && !switchPressed) {
                    switchPressed = true;
                    isPressing = false;
                }
            }
        } else {
            isPressing = false;
        }
        sleep_ms(DOOR_POLL_INTERVAL_MS);
    }
}

void control_loop() {
    while (true) {
        float temperature = max6675_read_temperature();
        if (temperature < 0) {
            sleep_ms(MAIN_LOOP_INTERVAL_MS);
            continue;
        }

        if (temperature >= TEMP_HIGHHIGH) {
            set_relay_highhigh(true);
            highHighCondition = true;
            startupCondition = false;
            forceHeatBySwitch = false;
            emberPreservationActive = false;

        } else if (highHighCondition && temperature <= TEMP_HIGH) {
            set_relay_highhigh(false);
            highHighCondition = false;

        } else if (switchPressed && !startupCondition) {
            set_relay_forceheat(true);
            forcedHeatActive = true;
            startupCondition = true;
            forceHeatBySwitch = true;
            switchPressed = false;
            emberPreservationActive = false;

        } else if (startupCondition && temperature >= TEMP_HIGH) {
            set_relay_forceheat(false);
            forcedHeatActive = false;
            startupCondition = false;
            forceHeatBySwitch = false;

        } else if (temperature < TEMP_LOWLOW && !forcedHeatActive && !startupCondition && !emberPreservationActive) {
            set_relay_forceheat(true);
            forcedHeatActive = true;
            forceHeatStart = get_absolute_time();

        } else if (forcedHeatActive && !forceHeatBySwitch &&
                   absolute_time_diff_us(forceHeatStart, get_absolute_time()) >= FORCE_HEAT_TIMEOUT_MS * 1000 &&
                   temperature < TEMP_LOWLOW) {
            set_relay_forceheat(false);
            forcedHeatActive = false;
            emberPreservationActive = true;

        } else if (forcedHeatActive && !startupCondition &&
                   temperature >= TEMP_LOW && temperature < TEMP_HIGH) {
            set_relay_forceheat(false);
            forcedHeatActive = false;
            emberPreservationActive = false;
        }

        send_temperature_data(temperature, forcedHeatActive, highHighCondition, startupCondition, emberPreservationActive);
        sleep_ms(MAIN_LOOP_INTERVAL_MS);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(5000);

    max6675_init();
    relays_init();
    wifi_init();

    gpio_init(DOOR_SWITCH_PIN);
    gpio_set_dir(DOOR_SWITCH_PIN, GPIO_IN);
    gpio_pull_down(DOOR_SWITCH_PIN);

    multicore_launch_core1(monitor_door_switch);
    control_loop();

    return 0;
}
