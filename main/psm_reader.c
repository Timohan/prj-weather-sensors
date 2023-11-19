/*!
 * \file
 * \brief file psm_reader.c
 *
 * pms5003 Reader
 * https://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf
 * Default baud rate：9600bps Check bit：None Stop bit：1 bit
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */
#include "psm_reader.h"
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "tcpip_sender.h"

static const int RX_BUF_SIZE = 1024*2;
#define RX_DATA_BUF_SIZE 2056

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

#define UART UART_NUM_2


static uint8_t m_psmData[RX_DATA_BUF_SIZE];
static uint32_t m_psmDataIndex = 0;
static struct PMSData m_psmParsedData;

void psm_init(void) 
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_set_wakeup_threshold(UART, 3);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

static bool psm_setParticles(uint8_t *psmData, uint32_t size)
{
    uint16_t buffer_u16[15];
    if (size < 30) {
        return false;
    }
    for (uint8_t i = 0; i < 15; i++) {
        buffer_u16[i] = psmData[1 + i * 2];
        buffer_u16[i] += (psmData[i * 2] << 8);
    }

    memcpy(&m_psmParsedData, (void *)buffer_u16, 30);
/*    printf ("%d %d %d %d . %d %d %d %d %d %d\n", m_psmParsedData.framelen,
        m_psmParsedData.pm10_standard, m_psmParsedData.pm25_standard, m_psmParsedData.pm100_standard,
        m_psmParsedData.particles_03um, m_psmParsedData.particles_05um, m_psmParsedData.particles_10um,
        m_psmParsedData.particles_25um, m_psmParsedData.particles_50um, m_psmParsedData.particles_100um
    );*/

    tcpip_setNewValue(SensorTypePM10, (double)(m_psmParsedData.pm10_standard));
    tcpip_setNewValue(SensorTypePM25, (double)(m_psmParsedData.pm25_standard));
    tcpip_setNewValue(SensorTypePM100, (double)(m_psmParsedData.pm100_standard));
    return true;
}

static void psm_removeByteFromBegin()
{
    uint32_t i;
    for (i=1;i<m_psmDataIndex;i++) {
        m_psmData[i-1] = m_psmData[i];
    }
    m_psmDataIndex--;
}

static void psm_removeBytesFromBegin(uint32_t count)
{
    uint32_t i;
    for (i=1;i<count;i++) {
        psm_removeByteFromBegin();
    }
}


static void psm_parseData()
{
    if (m_psmData[0] == FIXED_CHAR0 && m_psmData[1] == FIXED_CHAR1) {
        if (psm_setParticles(m_psmData+2, m_psmDataIndex-2)) {
            psm_removeBytesFromBegin(32);
        }
    } else {
        psm_removeByteFromBegin();
    }
}

void psm_reader(void)
{
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int i;
    size_t length = 0;
    while (1) {
        if (uart_get_buffered_data_len(UART, &length) == ESP_OK && length >= 32) {
            if (length > RX_BUF_SIZE) {
                length = RX_BUF_SIZE;
            }
            const int rxBytes = uart_read_bytes(UART, data, length, 100);
            if (rxBytes > 0) {
                data[rxBytes] = 0;
                for (i=0;i<rxBytes;i++) {
                    m_psmData[m_psmDataIndex++] = data[i];
                 }
                 while (1) {
                     if (m_psmDataIndex > 32) {
                        psm_parseData();
                     } else {
                        break;
                     }
                 }
            }
        }
        vTaskDelay(1);
    }
    free(data);
}