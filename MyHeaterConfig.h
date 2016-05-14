#pragma once
#ifndef _MY_HEATER_CONFIG_H_
#define _MY_HEATER_CONFIG_H_

#include <Maintain/Maintain.h>

class MyHeaterConfig {
    public:
        MyHeaterConfig(
            const char *version,
            uint32_t    updateInterval
        ) :
            _version(version),
            _updateInterval(updateInterval),
            _lastUpdate(0)
        {};

        void load(MaintainedCollection *heaters);
        void update(MaintainedCollection *heaters, bool force);

    private:
        const char *_version;
        uint32_t    _updateInterval;
        uint32_t    _lastUpdate;
};

#endif
