/*!
 * \file
 * \brief file prj_main.c
 *
 * Project main file
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "psm_reader.h"
#include "wifi_connect.h"
#include "bme280_reader.h"
#include "tcpip_sender.h"

void psm_task(void *arg) {
    psm_init();
    printf("psm start reading\n");
    psm_reader();
    vTaskDelay(1);
    printf("psm reading failed somehow, this should not happen\n");
    fflush(stdout);
    esp_restart();
}

void bme280_task(void *arg) {
    bme280_reader_init();
    bme280_reader_task();
    vTaskDelay(1);
    printf("bme280 reading failed somehow, this should not happen\n");
    fflush(stdout);
    esp_restart();
}

void tcpip_sender_task(void *arg) {
    tcpip_sender_init();
    printf("tcp/ip sending failed somehow, this should not happen\n");
    fflush(stdout);
    esp_restart();
}

void app_main(void)
{
    printf("Start prj-weather-sensor!\n");
    wifi_connect();
    xTaskCreate(&tcpip_sender_task, "tcpip_sender_task", 2048, NULL, 3, NULL);
    xTaskCreate(&bme280_task, "bme280_task", 2048, NULL, 10, NULL);
    xTaskCreatePinnedToCore(&psm_task, "psm_task", 2048, NULL, 10, NULL, 1);
}
