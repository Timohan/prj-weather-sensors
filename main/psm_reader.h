/*!
 * \file
 * \brief file psm_reader.h
 *
 * pms5003 Reader
 * https://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf
 * Default baud rate：9600bps Check bit：None Stop bit：1 bit
 *
 * Copyright of Timo Hannukkala. All rights reserved.
 *
 * \author Timo Hannukkala <timohannukkala@hotmail.com>
 */
#ifndef PSM_READER_H
#define PSM_READER_H

#define FIXED_CHAR0 0x42
#define FIXED_CHAR1 0x4d

#include <inttypes.h>

struct PMSData {
  uint16_t framelen;       ///< How long this data chunk is
  uint16_t pm10_standard,  ///< Standard PM1.0
      pm25_standard,       ///< Standard PM2.5
      pm100_standard;      ///< Standard PM10.0
  uint16_t pm10_env,       ///< Environmental PM1.0
      pm25_env,            ///< Environmental PM2.5
      pm100_env;           ///< Environmental PM10.0
  uint16_t particles_03um, ///< 0.3um Particle Count
      particles_05um,      ///< 0.5um Particle Count
      particles_10um,      ///< 1.0um Particle Count
      particles_25um,      ///< 2.5um Particle Count
      particles_50um,      ///< 5.0um Particle Count
      particles_100um;     ///< 10.0um Particle Count
  uint16_t unused;         ///< Unused
  uint16_t checksum;       ///< Packet checksum
};

void psm_init();
void psm_reader();

#endif // PSM_READER_H