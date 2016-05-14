#pragma once
#ifndef _MY_HEATER_TIME_H_
#define _MY_HEATER_TIME_H_

#include <Ethernet.h>
#include <EthernetUdp.h>

class MyHeaterTime {
    public:
        MyHeaterTime(
            IPAddress *server,
            uint32_t   firstSyncInterval,
            uint32_t   syncInterval,
            uint16_t   localPort,
            uint8_t    timeZone
        ) :
            _server(server),
            _firstSyncInterval(firstSyncInterval),
            _syncInterval(syncInterval),
            _localPort(localPort),
            _timeZone(timeZone),
            _nextSync(0),
            _firstSync(true)
        {};

        void maintain();

    private:
        IPAddress *_server;
        uint32_t   _firstSyncInterval;
        uint32_t   _syncInterval;
        uint16_t   _localPort;
        uint8_t    _timeZone;
        uint32_t   _nextSync;
        bool       _firstSync;
};

#endif
