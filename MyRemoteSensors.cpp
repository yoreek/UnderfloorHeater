#include <StringUtil.h>
#include "MyRemoteSensors.h"

void MyBedroomRemoteSensor::maintain() {
    if (   _updated     != 0
        && _dataLogger  != NULL
        && _logInterval != 0
        && _nextLogging <= millis()) {

        _dataLogger->log((char *) "%F %T,%.1f,%.1f,%.1f", getTemperature(), getHumidity(), getDewPoint());

        _nextLogging = millis() + _logInterval * 1000;
    }
}

void MyBedroomRemoteSensor::onRecv(RF24 *radio) {
    memset(&_data, 1, sizeof(_data));
    DEBUG("read");
    radio->read(&_data, sizeof(_data));
    _updated = now();
    DEBUG("recieved temp: %f humd: %f dewPoint: %f", _data.temp, _data.humd, _data.dewPoint);
}

void MyBedroomRemoteSensor::printJson(Print &out) {
    StringUtil::fprintf(out, F("{"
        "\"name\":\"%s\","
        "\"temperature\":%.1f,"
        "\"humidity\":%.1f,"
        "\"dewPoint\":%.1f,"
        "\"updated\":%ul"
        "}"),
        name(),
        getTemperature(),
        getHumidity(),
        getDewPoint(),
        updated());
}
