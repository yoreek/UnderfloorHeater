#include "MyHeater.h"

void MyHeater::maintain() {
    _net->maintain();

    if (_net->isConnected()) {
        _time->maintain();
        _ui->maintain();
    }

    _heaters->maintain();
    _remoteSensors->maintain();

    updateConfig(false);
}

void MyHeater::loadConfig() {
    _config->load(_heaters);
}

void MyHeater::updateConfig(bool force) {
    _config->update(_heaters, force);
}
