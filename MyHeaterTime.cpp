#include <Ethernet.h>
#include <EthernetUdp.h>
#include <DebugUtil.h>
#include <Time.h>
#include <Network/NtpClient.h>
#include "MyHeaterTime.h"

#ifndef _UDP_SOCK_NUM
#define _UDP_SOCK_NUM 0
#endif

void MyHeaterTime::maintain() {
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
