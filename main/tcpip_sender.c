/*!
 * \file
 * \brief file tcpip_sender.c
 *
 * sends BME280 & PSM Data to server
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#include "tcpip_sender.h"
#include <sys/socket.h>
#include <pthread.h>
#include "wifi_connect.h"
#include "default_values.h"

static ClientSideValue m_clientSide[SensorTypeNA];
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t m_nextItemToSend;

void tcpip_setNewValue(SensorType type, double value)
{
    pthread_mutex_lock(&m_mutex);
    m_clientSide[type].m_sent = false;
    m_clientSide[type].m_value = value;
    pthread_mutex_unlock(&m_mutex);
}

static char tcpip_getSensorTypeChar(SensorType type)
{
    switch (type) {
        case SensorTypeHumid: return 'h';
        case SensorTypeTemperature: return 't';
        case SensorTypePresure: return 'p';
        case SensorTypePM10: return 'a';
        case SensorTypePM25: return 'b';
        case SensorTypePM100: return 'c';
        case SensorTypeNA:
        default:
            break;
    }

    return ' ';
}

static void tcpip_printLogValue(const char *buffer, bool sentOk)
{
    printf("OK? %d : %s", (int)(sentOk), buffer);
}


static bool tcpip_sendValue(int sockClient, ClientSideValue *value)
{
    size_t i, c;
    char buffer[124];
    memset(buffer, '\0', 124*sizeof(char));
    snprintf(buffer, 120, "I%c%f", tcpip_getSensorTypeChar(value->m_type), value->m_value);

    c = strlen(buffer);
    for (i=0;i<c;i++) {
        if (buffer[i] == ',') {
            buffer[i] = '.';
        }
    }
    while (1) {
        c = strlen(buffer);
        if (c <= 5) {
            break;
        }
        if (buffer[c-1] == '0' && buffer[c-2] != '.') {
            buffer[c-1] = '\0';
            continue;
        }
        break;
    }
    strcat(buffer, "\n");
    c = strlen(buffer);

    if (send(sockClient, buffer, c, 0) == (int)(c)) {
        value->m_sent = true;
        tcpip_printLogValue(buffer, true);
        return true;
    }

    tcpip_printLogValue(buffer, false);
    return false;
}

static bool tcpip_setUp()
{
    int tcp_fail_count = 0;
    bool sent = false;
    size_t i;
    int sock_cli;

    sock_cli = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TCP_IP_PORT);
    servaddr.sin_addr.s_addr = inet_addr(TCP_IP_ADDR);

    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("Connecting to server failed\n");
        return false;
    }

    struct timeval tm;
    tm.tv_sec = 1;
    tm.tv_usec = 0;

    int err = setsockopt(sock_cli, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm));
    if (err < 0) {
        close(sock_cli);
        return false;
    }

    printf("Connected to server\n" );
    vTaskDelay(500/portTICK_PERIOD_MS);

    while(1){
        vTaskDelay(500/portTICK_PERIOD_MS);
        sent = false;
        pthread_mutex_lock(&m_mutex);
        for (i=m_nextItemToSend;i<(size_t)(SensorTypeNA);i++) {
            if (m_clientSide[i].m_sent == false) {
                if (tcpip_sendValue(sock_cli, &m_clientSide[i])) {
                    tcp_fail_count = 0;
                } else {
                    tcp_fail_count++;
                }
                sent = true;
                m_nextItemToSend++;
                break;
            }
        }
        if (!sent) {
            for (i=0;i<(size_t)(m_nextItemToSend);i++) {
                if (m_clientSide[i].m_sent == false) {
                    if (tcpip_sendValue(sock_cli, &m_clientSide[i])) {
                        tcp_fail_count = 0;
                    } else {
                        tcp_fail_count++;
                    }
                    sent = true;
                    m_nextItemToSend++;
                    break;
                }
            }
        }
        if (m_nextItemToSend == (size_t)(SensorTypeNA)) {
            m_nextItemToSend = 0;
        }

        pthread_mutex_unlock(&m_mutex);
        if (tcp_fail_count > 10 || wifi_connect_get_connected() == 0) {
            break;
        }
    }

    close(sock_cli);
    return true;
}

void tcpip_sender_init()
{
    printf("tcp sender init().\n");
    m_nextItemToSend = 0;
    size_t i;
    for (i=0;i<(size_t)(SensorTypeNA);i++) {
        m_clientSide[i].m_type = (SensorType)(i);
        m_clientSide[i].m_sent = true;
        m_clientSide[i].m_value = 0;
    }
    printf("tcp sender init() setup");
    while (1) {
        vTaskDelay(1);
        if (wifi_connect_get_connected() == 0) {
            printf("wifi not connected\n");
            vTaskDelay(500);
            continue;
        }
        printf("tcp sender init()x setup");
        tcpip_setUp();
        vTaskDelay(500/ portTICK_PERIOD_MS);
    }
}