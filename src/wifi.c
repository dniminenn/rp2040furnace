#include <stdio.h>
#include <string.h>

#include <cyw43_country.h>
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"

#include "config.h"
#include "wifi.h"

bool wifi_check_connectivity() {
    struct netif *netif = netif_list;
    if (!netif || ip4_addr_isany_val(*netif_ip4_gw(netif))) {
        printf("No gateway configured. Cannot check connectivity.\n");
        return false;
    }

    ip4_addr_t gateway = *netif_ip4_gw(netif);
    char gateway_ip[16];
    ip4addr_ntoa_r(&gateway, gateway_ip, sizeof(gateway_ip));

    printf("Checking connectivity to gateway: %s\n", gateway_ip);

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Failed to create TCP PCB\n");
        return false;
    }

    err_t result = tcp_connect(pcb, &gateway, 80, NULL);
    tcp_close(pcb);

    if (result == ERR_OK) {
        printf("Connectivity check successful (Raw TCP connect to gateway)\n");
        return true;
    } else {
        printf("Connectivity check failed: %d\n", result);
        return false;
    }
}

void wifi_init() {
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_CANADA)) {
        printf("Wi-Fi initialization failed: Hardware init error\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    wifi_connect();
}

void wifi_connect() {
    if (wifi_check_connectivity()) {
        printf("Wi-Fi already connected\n");
        return;
    }
    printf("Attempting to connect to Wi-Fi SSID: %s\n", WIFI_SSID);
    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000);
    if (result == 0) {
        printf("Wi-Fi connected successfully\n");
    } else {
        printf("Wi-Fi connection failed: %d\n", result);
    }
}

void send_temperature_data(float temperature, const FurnaceState *state) {
    if (!wifi_check_connectivity()) {
        printf("Connectivity to gateway failed. Skipping data transmission.\n");
        return;
    }

    printf("Sending temperature data to receiver\n");
    struct udp_pcb *pcb = udp_new();
    if (!pcb) return;

    ip_addr_t dest_ip;
    ip4addr_aton(RECEIVER_IP, &dest_ip);

    char buffer[256];
    int len = snprintf(buffer, sizeof(buffer),
        "{\"temperature\":%.1f,\"forcedHeatActive\":%s,\"highHighCondition\":%s,"
        "\"startupCondition\":%s,\"emberPreservationActive\":%s}",
        temperature,
        state->forcedHeatActive ? "true" : "false",
        state->highHighCondition ? "true" : "false",
        state->startupCondition ? "true" : "false",
        state->emberPreservationActive ? "true" : "false");

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (!p) {
        udp_remove(pcb);
        return;
    }

    memcpy(p->payload, buffer, len);

    udp_sendto(pcb, p, &dest_ip, RECEIVER_PORT);
    pbuf_free(p);
    udp_remove(pcb);
}
