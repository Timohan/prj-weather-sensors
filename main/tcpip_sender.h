/*!
 * \file
 * \brief file tcpip_sender.h
 *
 * sends BME280 & PSM Data to server
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */

#ifndef TCPIP_SENDER_H
#define TCPIP_SENDER_H

#define BUFFER_SIZE 1024

#include <inttypes.h>
#include <stdbool.h>

typedef enum
{
    SensorTypeTemperature = 0,
    SensorTypeHumid,
    SensorTypePresure,
    SensorTypePM10,
    SensorTypePM25,
    SensorTypePM100,
    SensorTypeNA,
} SensorType;

typedef struct
{
    double m_value;
    SensorType m_type;
    bool m_sent;
} ClientSideValue;

void tcpip_sender_init();
void tcpip_setNewValue(SensorType type, double value);

#endif // TCPIP_SENDER_H