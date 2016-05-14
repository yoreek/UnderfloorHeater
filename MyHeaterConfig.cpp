#include <EEPROM.h>
#include <Heater/UnderfloorHeater.h>
#include <DebugUtil.h>
#include "MyHeaterConfig.h"

typedef struct _HeaterConfig HeaterConfig;
struct _HeaterConfig {
    uint32_t              runningTime;
    float                 targetTemperature;
    ScheduledHeater::Mode mode;
};

typedef struct _HeaterPeriodConfig HeaterPeriodConfig;
struct _HeaterPeriodConfig {
    int end;
    int setpoint;
};

void MyHeaterConfig::load(MaintainedCollection *heaters) {
    int                 i, d, z;
    size_t              j, version_len, offset, start;
    HeaterConfig        heaterConfig;
    HeaterPeriodConfig  heaterPeriodConfig;
    UnderfloorHeater   *heater;
    EnergyMeter        *energyMeter;
    char               *p;
    MaintainedObject   *obj;

    DEBUG("load config");

    offset = 0;
    version_len = strlen(_version);
    for (j = 0; j < version_len; j++) {
        if (EEPROM.read(offset + j) != _version[j]) {
            DEBUG("no config");

            return;
        }
    }
    offset += version_len;

    i = 0;
    obj = heaters->first();
    while (obj != NULL) {
        heater      = (UnderfloorHeater *) obj;
        energyMeter = heater->getEnergyMeter();

        p = (char *) &heaterConfig;
        for (j = 0; j < sizeof(HeaterConfig); j++) {
            *(p + j) = EEPROM.read(offset + j);
        }
        offset += sizeof(HeaterConfig);

        energyMeter->setRunningTime(heaterConfig.runningTime);
        heater->setTargetTemperature(heaterConfig.targetTemperature);
        heater->setMode(heaterConfig.mode);

        p = (char *) &heaterPeriodConfig;
        for (d = 0; d < SCHEDULE_HEATER_MAX_DAYS; d++) {
            start = 0;
            for (z = 0; z < SCHEDULE_HEATER_MAX_ZONES; z++) {
                for (j = 0; j < sizeof(HeaterPeriodConfig); j++) {
                    *(p + j) = EEPROM.read(offset + j);
                }
                offset += sizeof(HeaterPeriodConfig);

                heater->schedule[d][z].start    = start;
                heater->schedule[d][z].end      = heaterPeriodConfig.end;
                heater->schedule[d][z].setpoint = (float ) heaterPeriodConfig.setpoint / 100.0;
                start = heaterPeriodConfig.end;
            }
        }

        obj = obj->next;
        i++;
    }

    DEBUG("loaded");
}

void MyHeaterConfig::update(MaintainedCollection *heaters, bool force) {
    unsigned long       now;
    int                 i, d, z;
    size_t              j, version_len, offset;
    HeaterConfig        heaterConfig;
    HeaterPeriodConfig  heaterPeriodConfig;
    char               *p;
    UnderfloorHeater   *heater;
    EnergyMeter        *energyMeter;
    MaintainedObject   *obj;

    now = millis();
    if (!force && _lastUpdate != 0 && now >= _lastUpdate && (now - _lastUpdate) < _updateInterval * 1000) {
        return;
    }

    DEBUG("update config");

    _lastUpdate = now;
    offset = 0;

    version_len = strlen(_version);
    for (j = 0; j < version_len; j++) {
        if (EEPROM.read(offset + j) != _version[j]) {
            EEPROM.write(offset + j, _version[j]);
        }
    }
    offset += version_len;

    i = 0;
    obj = heaters->first();
    while (obj != NULL) {
        heater      = (UnderfloorHeater *) obj;
        energyMeter = heater->getEnergyMeter();

        p = (char *) &heaterConfig;
        heaterConfig.runningTime       = energyMeter->getRunningTime();
        heaterConfig.targetTemperature = heater->getTargetTemperature();
        heaterConfig.mode              = heater->getMode();

        for (j = 0; j < sizeof(HeaterConfig); j++) {
            if (EEPROM.read(offset + j) != *(p + j)) {
                EEPROM.write(offset + j, *(p + j));
            }
        }
        offset += sizeof(HeaterConfig);

        p = (char *) &heaterPeriodConfig;
        for (d = 0; d < SCHEDULE_HEATER_MAX_DAYS; d++) {
            for (z = 0; z < SCHEDULE_HEATER_MAX_ZONES; z++) {
                heaterPeriodConfig.end      = heater->schedule[d][z].end;
                heaterPeriodConfig.setpoint = lround(heater->schedule[d][z].setpoint * 100.0);

                for (j = 0; j < sizeof(HeaterPeriodConfig); j++) {
                    if (EEPROM.read(offset + j) != *(p + j)) {
                        EEPROM.write(offset + j, *(p + j));
                    }
                }
                offset += sizeof(HeaterPeriodConfig);
            }
        }

        obj = obj->next;
        i++;
    }
}
