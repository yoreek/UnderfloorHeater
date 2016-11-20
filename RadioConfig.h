#pragma once
#ifndef _RADIO_CONFIG_H_
#define _RADIO_CONFIG_H_

#include <RF24.h>

#define RADIO_SERVER_ADDR      0x7878787878LL
#define RADIO_CHILD_BASE_ADDR  0xA3A3A3A300LL
#define RADIO_BEDROOM_ADDR     (RADIO_CHILD_BASE_ADDR | 1)
#define RADIO_BATHROOM_ADDR    (RADIO_CHILD_BASE_ADDR | 2)
#define RADIO_MAX_PIPES        6

#define RADIO_PA_LEVEL         RF24_PA_MIN
#define RADIO_DATA_RATE        RF24_1MBPS
#define RADIO_CHANNEL          33  //Pick a channel to use between 0 nd 70
#define RADIO_IRQ_PIN          2
#define RADIO_CE_PIN           9
#define RADIO_CS_PIN           10

#endif
