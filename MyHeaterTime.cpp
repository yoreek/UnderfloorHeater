#include <Ethernet.h>
#include <EthernetUdp.h>
#include <DebugUtil.h>
#include <Time.h>
#include <Network/NtpClient.h>
#include "MyHeaterTime.h"

#ifndef _UDP_SOCK_NUM
#define _UDP_SOCK_NUM 0
#endif

void MyHeaterTime::syncTime() {
    time_t             time;
    EthernetUDP        udp;

    if (millis() < _nextSync) return;

    DEBUG("udp.begin()");
#ifdef UDP_SELECT_CUSTOM_SOCKET
    if (!udp.begin(_localPort, _UDP_SOCK_NUM)) {
        DEBUG("udp.begin() failed");
        goto FINISH;
    }
#else
    if (!udp.begin(_localPort)) {
        DEBUG("udp.begin() failed");
        goto FINISH;
    }
#endif

    DEBUG("get time");
    time = NtpClient::getTime(udp, *_server, _timeZone);
    if (time == 0) {
        DEBUG("Failed to update time");
        goto STOP_UDP;
    }

    setTime(time);
    DEBUG("set time to: %F %T");
    _firstSync = false;

STOP_UDP:
    udp.stop();
FINISH:
    _nextSync = millis() + (_firstSync ? _firstSyncInterval : _syncInterval) * 1000;
}

void MyHeaterTime::broadcastTime() {
    if (_radio == NULL)            return;
    if (_firstSync)                return;
    if (millis() < _nextBroadcast) return;

    DEBUG("broadcast current time");
    time_t t = now();
    _radio->stopListening();
    if (!_radio->write(&t, sizeof(t), 1)) {
        DEBUG("Sending failed.");
    }
    _radio->startListening();

    _nextBroadcast = millis() + _broadcastInterval * 1000;
}

void MyHeaterTime::maintain() {
    syncTime();
    broadcastTime();
}
