#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"

#include "config.h"

void max6675_init() {
    spi_init(SPI_PORT, SPI_CLOCK_SPEED);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}

float max6675_read_temperature() {
    uint8_t buffer[2];
    uint16_t value;

    gpio_put(PIN_CS, 0);
    sleep_us(10);

    spi_read_blocking(SPI_PORT, 0x00, buffer, 2);

    gpio_put(PIN_CS, 1);

    value = (buffer[0] << 8) | buffer[1];
    if (value & 0x4) return -1.0;

    value >>= 3;
    return value * 0.25;
}
