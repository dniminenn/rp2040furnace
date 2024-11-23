#include <stdio.h>

#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "config.h"
#include "max6675.h"
#include "relays.h"
#include "state.h"
#include "wifi.h"

FurnaceState furnaceState = {0};
mutex_t state_lock;

void monitor_door_switch() {
    absolute_time_t pressStart;
    bool isPressing = false;

    uint32_t last_wifi_check = to_ms_since_boot(get_absolute_time());
    const uint32_t wifi_check_interval = 60000;

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_wifi_check >= wifi_check_interval) {
            last_wifi_check = now;
            wifi_connect();
        }

        mutex_enter_blocking(&state_lock);
        bool localStartupCondition = furnaceState.startupCondition;
        mutex_exit(&state_lock);

        if (localStartupCondition) {
            sleep_ms(DOOR_POLL_INTERVAL_MS);
            continue;
        }

        bool switchState = gpio_get(DOOR_SWITCH_PIN);
        if (switchState) {
            if (!isPressing) {
                isPressing = true;
                pressStart = get_absolute_time();
            } else {
                int64_t pressDuration = absolute_time_diff_us(pressStart, get_absolute_time());
                if (pressDuration >= DOOR_OPEN_TIME_MS * 1000) {
                    mutex_enter_blocking(&state_lock);
                    if (!furnaceState.switchPressed) {
                        furnaceState.switchPressed = true;
                    }
                    mutex_exit(&state_lock);
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

        mutex_enter_blocking(&state_lock);
        FurnaceState localState = furnaceState;
        mutex_exit(&state_lock);

        if (temperature >= TEMP_HIGHHIGH) {
            set_relay_highhigh(true);

            mutex_enter_blocking(&state_lock);
            furnaceState.highHighCondition = true;
            furnaceState.startupCondition = false;
            furnaceState.forceHeatBySwitch = false;
            furnaceState.emberPreservationActive = false;
            furnaceState.switchPressed = false;
            mutex_exit(&state_lock);

        } else if (localState.highHighCondition && temperature <= TEMP_HIGH) {
            set_relay_highhigh(false);

            mutex_enter_blocking(&state_lock);
            furnaceState.highHighCondition = false;
            mutex_exit(&state_lock);

        } else if (localState.switchPressed && !localState.startupCondition) {
            set_relay_forceheat(true);

            mutex_enter_blocking(&state_lock);
            furnaceState.forcedHeatActive = true;
            furnaceState.startupCondition = true;
            furnaceState.forceHeatBySwitch = true;
            furnaceState.emberPreservationActive = false;
            furnaceState.switchPressed = false;
            mutex_exit(&state_lock);

        } else if (localState.startupCondition && temperature >= TEMP_HIGH) {
            set_relay_forceheat(false);

            mutex_enter_blocking(&state_lock);
            furnaceState.forcedHeatActive = false;
            furnaceState.startupCondition = false;
            furnaceState.forceHeatBySwitch = false;
            furnaceState.switchPressed = false;
            mutex_exit(&state_lock);

        } else if (temperature < TEMP_LOWLOW && !localState.forcedHeatActive &&
                   !localState.startupCondition && !localState.emberPreservationActive) {
            set_relay_forceheat(true);

            mutex_enter_blocking(&state_lock);
            furnaceState.forcedHeatActive = true;
            furnaceState.forceHeatStart = get_absolute_time();
            mutex_exit(&state_lock);

        } else if (localState.forcedHeatActive && !localState.forceHeatBySwitch &&
                   absolute_time_diff_us(localState.forceHeatStart, get_absolute_time()) >= FORCE_HEAT_TIMEOUT_MS * 1000 &&
                   temperature < TEMP_LOWLOW) {
            set_relay_forceheat(false);

            mutex_enter_blocking(&state_lock);
            furnaceState.forcedHeatActive = false;
            furnaceState.emberPreservationActive = true;
            mutex_exit(&state_lock);

        } else if (localState.forcedHeatActive && !localState.startupCondition &&
                   temperature >= TEMP_LOW && temperature < TEMP_HIGH) {
            set_relay_forceheat(false);

            mutex_enter_blocking(&state_lock);
            furnaceState.forcedHeatActive = false;
            furnaceState.emberPreservationActive = false;
            mutex_exit(&state_lock);
        }

        send_temperature_data(temperature, &localState);
        sleep_ms(MAIN_LOOP_INTERVAL_MS);
    }
}

int main() {
    stdio_init_all();

    max6675_init();
    relays_init();
    wifi_init();

    gpio_init(DOOR_SWITCH_PIN);
    gpio_set_dir(DOOR_SWITCH_PIN, GPIO_IN);
    gpio_pull_down(DOOR_SWITCH_PIN);

    mutex_init(&state_lock);

    multicore_launch_core1(monitor_door_switch);
    control_loop();

    return 0;
}
