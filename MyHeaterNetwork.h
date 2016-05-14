#pragma once
#ifndef _MY_HEATER_NETWORK_H_
#define _MY_HEATER_NETWORK_H_

#if defined(ARDUINO) && ARDUINO >= 100
#  include "Arduino.h"
#else
#  include "WProgram.h"
#endif

class MyHeaterNetwork {
    public:
        MyHeaterNetwork(
            uint8_t *macAddress,
#ifdef WITH_ICMP
            uint8_t *pingServer,
#endif
            uint32_t checkInterval
        ) :
            _macAddress(macAddress),
#ifdef WITH_ICMP
            _pingServer(pingServer),
#endif
            _firstConnection(true),
            _connected(false),
            _checkInterval(checkInterval),
            _lastCheck(0)
        {};

        void maintain();
        bool isConnected();

    protected:
        uint8_t  *_macAddress;
#ifdef WITH_ICMP
        uint8_t  *_pingServer;
#endif
        bool      _firstConnection;
        bool      _connected;
        uint32_t  _checkInterval;
        uint32_t  _lastCheck;
};

#endif
