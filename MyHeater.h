#pragma once
#ifndef _MY_HEATER_H_
#define _MY_HEATER_H_

#include <Maintain/Maintain.h>
#include "MyHeaterConfig.h"
#include "MyHeaterNetwork.h"
#include "MyHeaterTime.h"
#include "MyHeaterUI.h"

class MyHeater {
    public:
        MyHeater(
            MyHeaterConfig             *config,
            MyHeaterNetwork            *net,
            MyHeaterTime               *time,
            MyHeaterUI                 *ui,
            MaintainedCollection       *heaters,
            MaintainedCollectionSimple *remoteSensors
        ) :
            _config(config),
            _net(net),
            _time(time),
            _ui(ui),
            _heaters(heaters),
            _remoteSensors(remoteSensors)
        {
            loadConfig();
        }

        void loadConfig();
        void updateConfig(bool force);
        void maintain();
        MaintainedCollection *heaters(void) {
            return _heaters;
        };
        MaintainedCollectionSimple *remoteSensors(void) {
            return _remoteSensors;
        };

    private:
        MyHeaterConfig             *_config;
        MyHeaterNetwork            *_net;
        MyHeaterTime               *_time;
        MyHeaterUI                 *_ui;
        MaintainedCollection       *_heaters;
        MaintainedCollectionSimple *_remoteSensors;
};

#endif
