#if defined(WITH_DEBUG) && ! defined(WITH_NETWORK_DEBUG)
#  undef WITH_DEBUG
#endif
#include <DebugUtil.h>
#include <Ethernet.h>
#ifdef WITH_ICMP
#include <ICMPPing.h>
#endif
#include "MyHeaterNetwork.h"

void MyHeaterNetwork::maintain() {
    uint32_t now;

    now = millis();
    if (!_firstConnection && now >= _lastCheck && (now - _lastCheck) < _checkInterval * 1000) {
        return;
    }

    _lastCheck = now;

    if (_firstConnection) {
        DEBUG("Initiate connection...");
        if ( Ethernet.begin(_macAddress) == 0 ) {
            DEBUG("Failed to configure Ethernet using DHCP");
            return;
        }
        else {
            DEBUG("Connected successfully");
#ifdef MY_HEATER_NETWORK_DEBUG
            Serial.print("IP: "); Serial.println(Ethernet.localIP());
#endif
            _firstConnection = false;
        }
    }
    else {
        DEBUG("Maintain connection...");
        switch ( Ethernet.maintain() ) {
            case 0:
                DEBUG("Nothing happened");
                break;
            case 1:
                DEBUG("Renew failed");
                break;
            case 2:
                DEBUG("Rebind fail");
                break;
            case 3:
                DEBUG("Rebind success");
                break;
            default:
                DEBUG("Unknown state");
        }
    }

#ifdef WITH_ICMP
    IPAddress  pingAddr(_pingServer);
    SOCKET     pingSocket        = 3;
    ICMPPing   ping(pingSocket, (uint16_t) random(0, 255));

    DEBUG("Send echo request...");

    ICMPEchoReply echoReply = ping(pingAddr, 4);
    if (echoReply.status == SUCCESS) {
#ifdef MY_HEATER_NETWORK_DEBUG
        Serial.print(F("Reply["));
        Serial.print(echoReply.data.seq);
        Serial.print(F("] from: "));
        Serial.print(echoReply.addr[0]);
        Serial.print(".");
        Serial.print(echoReply.addr[1]);
        Serial.print(".");
        Serial.print(echoReply.addr[2]);
        Serial.print(".");
        Serial.print(echoReply.addr[3]);
        Serial.print(F(": bytes="));
        Serial.print(REQ_DATASIZE);
        Serial.print(F(" time="));
        Serial.print(millis() - echoReply.data.time);
        Serial.print(F(" TTL="));
        Serial.println(echoReply.ttl);
#endif

        _connected = true;

        return;
    }

    DEBUG("Echo request failed: %s", echoReply.status);

    _connected = false;
#else
    _connected = true;
#endif
}

bool MyHeaterNetwork::isConnected() {
    return _connected;
}
