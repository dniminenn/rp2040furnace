#include <cyw43_country.h>
#include <stdio.h>
#include <string.h>
#include "pico/cyw43_arch.h"
#include "wifi.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "config.h"

void wifi_init() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_CANADA)) {
        printf("Wi-Fi initialization failed: Hardware init error\n");
        return;
    }

    cyw43_arch_enable_sta_mode();

    printf("Attempting to connect to Wi-Fi SSID: %s\n", WIFI_SSID);

    int max_retries = 4;
    int attempt = 0;
    int result;

    while (attempt < max_retries) {
        result = cyw43_arch_wifi_connect_timeout_ms("HADEVICE", "iotdevice11", CYW43_AUTH_WPA2_AES_PSK, 10000);
        if (result == 0) {
            printf("Wi-Fi connected successfully on attempt %d\n", attempt + 1);
            return;
        }

        switch (result) {
            case -1:
                printf("Wi-Fi connection failed: No SSID found (Attempt %d/%d)\n", attempt + 1, max_retries);
                break;
            case -2:
                printf("Wi-Fi connection failed: Authentication error (Attempt %d/%d)\n", attempt + 1, max_retries);
                break;
            case -3:
                printf("Wi-Fi connection failed: Timeout (Attempt %d/%d)\n", attempt + 1, max_retries);
                break;
            default:
                printf("Wi-Fi connection failed: Unknown error (%d) (Attempt %d/%d)\n", result, attempt + 1, max_retries);
                break;
        }

        attempt++;
        printf("Retrying in 5 seconds...\n");
        sleep_ms(5000);
    }

    printf("Wi-Fi connection failed after %d attempts\n", max_retries);
}

void send_temperature_data(float temperature, bool forcedHeatActive, bool highHighCondition, bool startupCondition, bool emberPreservationActive) {
    printf("Sending temperature data to receiver\n");
    struct udp_pcb* pcb = udp_new();
    if (!pcb) return;

    ip_addr_t dest_ip;
    ip4addr_aton(RECEIVER_IP, &dest_ip);

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer),
        "{\"temperature\":%.1f,\"forcedHeatActive\":%s,\"highHighCondition\":%s,\"startupCondition\":%s,\"emberPreservationActive\":%s}",
        temperature,
        forcedHeatActive ? "true" : "false",
        highHighCondition ? "true" : "false",
        startupCondition ? "true" : "false",
        emberPreservationActive ? "true" : "false"
    );

    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (!p) {
        udp_remove(pcb);
        return;
    }

    printf("Sending data: %s\n", buffer);

    memcpy(p->payload, buffer, len);

    udp_sendto(pcb, p, &dest_ip, RECEIVER_PORT);
    pbuf_free(p);
    udp_remove(pcb);
}
