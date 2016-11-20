#pragma once
#ifndef _MY_REMOTE_SENSORS_H_
#define _MY_REMOTE_SENSORS_H_

#include <RF24.h>
#include <DebugUtil.h>
#include <Maintain/Maintain.h>
#include <Logger/DataLogger.h>
#include "MyMulticeiver.h"

class MyRemoteSensor: public MyReceiver, public MaintainedObject {
    public:
        MyRemoteSensor(
            const char *name,
            DataLogger *dataLogger,
            uint32_t    logInterval
        ) :
            MyReceiver(name),
            MaintainedObject(),
            _dataLogger(dataLogger),
            _logInterval(logInterval),
            _nextLogging(0)
        {};
        virtual void printJson(Print &out) {};

    protected:
        DataLogger *_dataLogger;
        uint32_t    _logInterval;
        uint32_t    _nextLogging;
};

class MyBedroomRemoteSensor: public MyRemoteSensor {
    public:
        MyBedroomRemoteSensor(
            const char *name,
            DataLogger *dataLogger = NULL,
            uint32_t    logInterval = 0
        ) :
            MyRemoteSensor(name, dataLogger, logInterval),
            _data({0}),
            _updated(0)
        {};

        void onRecv(RF24 *radio);
        inline float getTemperature() {
            return _data.temp;
        };
        inline float getHumidity() {
            return _data.humd;
        };
        inline float getDewPoint() {
            return _data.dewPoint;
        };
        inline time_t updated() {
            return _updated;
        };
        void printJson(Print &out);
        void maintain();

    protected:
        struct {
            float temp;
            float humd;
            float dewPoint;
        } _data;
        time_t _updated;
};

#endif
