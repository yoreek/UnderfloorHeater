#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "RadioConfig.h"

#define WEBSERVER_PREFIX           ""
#define WEBSERVER_PORT             80
// base64 "admin:admin"
#define WEBSERVER_AUTH_CREDENTIALS "YWRtaW46YWRtaW4="

#define CONFIG_VERSION             "sh-2.0"

#define MAINTAIN_HEATERS_INTERVAL  10   // seconds
#define MAINTAIN_SENSORS_INTERVAL  10   // seconds
#define UPDATE_CONFIG_INTERVAL     3600 // seconds
#define LOG_INTERVAL               300  // seconds
#define LOG_HEADER                 ("Timestamp,Temperature,Consumption,Delta")

/* Ethernal shield uses 11-13 pins, 4 and 10 pins must be HIGH */
#define ETHERNET_SHIELD_SDCARD_PIN 4
#define ETHERNET_SHIELD_W5100_PIN  10
static byte MAC_ADDRESSS[]       = { 0x70, 0x5A, 0xB6, 0x01, 0x02, 0x03 };
#define CONNECTION_CHECK_INTERVAL  300 // seconds

#define NTP_SERVER                 132, 163, 4, 101
#define NTP_FIRST_SYNC_INTERVAL    30   // seconds
#define NTP_SYNC_INTERVAL          3600 // seconds
#define NTP_LOCAL_PORT             8887
#define NTP_TIME_ZONE              3
#define TIME_BROADCAST_INTERVAL    60 // seconds

#ifdef WITH_ICMP
#define PING_SERVER                8, 8, 8, 8
#endif

#define LOGGER_HOST                67, 214, 212, 103

#define SENSOR_SERIES_RESISTOR     1000
#define TERMISTOR_NOMINAL          10000
#define TERMISTOR_NOMINAL_HALL     6000
#define SENSOR_ADC_CORRECTION      -48
#define SENSOR_NUM_SAMPLES         10

#define KITCHEN_HEATER_POWER       776  // 840VA
#define BEDROOM_HEATER_POWER       1130 // 1250VA
#define HALL_HEATER_POWER          550  // 600VA

#define KITCHEN_HEATER_PIN         5
#define BEDROOM_HEATER_PIN         6
#define HALL_HEATER_PIN            7

#define KITCHEN_SENSOR_PIN         0
#define BEDROOM_SENSOR_PIN         1
#define HALL_SENSOR_PIN            2

#undef RADIO_CE_PIN
#undef RADIO_CS_PIN
#define RADIO_CE_PIN               8
#define RADIO_CS_PIN               9

#define BEDROOM_SENSOR_LOG_HEADER  ("Timestamp,Temperature,Humidity,DewPoint")

#endif
