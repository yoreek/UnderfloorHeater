#pragma once
#ifndef _MY_HEATER_TIME_H_
#define _MY_HEATER_TIME_H_

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <RF24.h>
#include <Timezone.h>

class MyHeaterTime {
    public:
        MyHeaterTime(
            IPAddress *server,
            uint32_t   firstSyncInterval,
            uint32_t   syncInterval,
            uint16_t   localPort,
            Timezone  *timeZone,
            RF24      *radio = NULL,
            uint32_t   broadcastInterval = 0
        ) :
            _server(server),
            _firstSyncInterval(firstSyncInterval),
            _syncInterval(syncInterval),
            _localPort(localPort),
            _timeZone(timeZone),
            _nextSync(0),
            _firstSync(true),
            _radio(radio),
            _broadcastInterval(broadcastInterval),
            _nextBroadcast(0)
        {};

        void syncTime();
        void broadcastTime();
        void maintain();

    private:
        IPAddress *_server;
        uint32_t   _firstSyncInterval;
        uint32_t   _syncInterval;
        uint16_t   _localPort;
        Timezone  *_timeZone;
        uint32_t   _nextSync;
        bool       _firstSync;
        RF24      *_radio;
        uint32_t   _broadcastInterval;
        uint32_t   _nextBroadcast;
};

#endif
