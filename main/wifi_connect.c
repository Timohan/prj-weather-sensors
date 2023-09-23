/*!
 * \file
 * \brief file wifi_connect.c
 *
 * Connects to WIFI station
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#include "wifi_connect.h"
#include <string.h>
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "default_values.h"

#define WIFI_MAXIMUM_RETRY      10
#define WIFI_CONNECTED_BIT      BIT0

static EventGroupHandle_t m_wifi_event_group;
static const char *TAG = "wifi connect";

static int m_wifi_connect_retry_count = 0;
static int m_wifi_connected = 0;

int wifi_connect_get_connected()
{
    return m_wifi_connected;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        m_wifi_connected = 0;
        if (m_wifi_connect_retry_count < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            m_wifi_connect_retry_count++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            // not sure if this is a good idea to re-start just like this
            printf("Restarting now because wifi not connected in enough time.\n");
            fflush(stdout);
            esp_restart();
        }
        printf("failed to connect to wifi ssid: %s, pw: %s\n", WIFI_SSID, WIFI_PASS);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        m_wifi_connect_retry_count = 0;
        m_wifi_connected = 1;
        xEventGroupSetBits(m_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    m_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    esp_wifi_set_ps(WIFI_PS_NONE);
    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(m_wifi_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        printf("connected to wifi ssid: %s pw: %s\n", WIFI_SSID, WIFI_PASS);
    } else {
        printf("Something else\n");
    }
}

void wifi_connect(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    wifi_init_sta();
}