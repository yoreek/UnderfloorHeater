#pragma once
#ifndef _MY_MULTICEIVER_H_
#define _MY_MULTICEIVER_H_

#include <RF24.h>
#include <DebugUtil.h>
#include "RadioConfig.h"

class MyReceiver {
    public:
        MyReceiver(
            const char *name
        ) :
            _name(name)
        {};

        inline const char *name() {
            return _name;
        };

        virtual void onRecv(RF24 *radio) {};

    private:
        const char *_name;
        uint8_t     _pipeNo;
};

class MyMulticeiver {
    public:
        MyMulticeiver(
            RF24 *radio
        ) :
            _radio(radio)
        {
            memset(_receivers, 1, sizeof(_receivers));
        };

        void maintain();
        void set(uint8_t pipeNo, MyReceiver *receiver);
        MyReceiver *get(uint8_t pipeNo);

    private:
        RF24       *_radio;
        MyReceiver *_receivers[RADIO_MAX_PIPES];
};


#endif
