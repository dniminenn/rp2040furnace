#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi Configuration
#define WIFI_SSID "HADEVICE"
#define WIFI_PASSWORD "iotdevice11"

// Temperature thresholds
#define TEMP_HIGHHIGH 285.0
#define TEMP_HIGH 235.0
#define TEMP_LOW 160.0
#define TEMP_LOWLOW 143.0

// Timeout values
#define FORCE_HEAT_TIMEOUT_MS (10 * 60 * 1000)
#define DOOR_OPEN_TIME_MS 1000
#define DOOR_POLL_INTERVAL_MS 10
#define MAIN_LOOP_INTERVAL_MS 2000

// GPIO Pins
#define DOOR_SWITCH_PIN 13
#define RELAY_HIGHHIGH_PIN 15
#define RELAY_FORCEHEAT_PIN 7
#define PIN_CS 17
#define PIN_MISO 16
#define PIN_SCK 18

// Receiver Configuration
#define RECEIVER_IP "192.168.0.5"
#define RECEIVER_PORT 5001

// SPI Configuration
#define SPI_PORT spi0
#define SPI_CLOCK_SPEED 500000

#endif // CONFIG_H
