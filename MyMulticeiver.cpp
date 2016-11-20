#include "MyMulticeiver.h"

void MyMulticeiver::maintain() {
    uint8_t     pipeNo;
    MyReceiver *receiver;

    while (_radio->available(&pipeNo)) {
        DEBUG("read from pipe: %d", (int) pipeNo);

        receiver = get(pipeNo);
        if (receiver == NULL) {
            DEBUG("Can't find receiver for pipe No: %d", (int) pipeNo);
#ifdef RF24_TUNED
            _radio->flush_rx();
#else
            char buf;
            while (_radio->available())
                _radio->read(&buf, sizeof(char));
#endif
        }
        else {
            DEBUG("Found receiver: %s", receiver->name());
            receiver->onRecv(_radio);
        }
    }
}

void MyMulticeiver::set(uint8_t pipeNo, MyReceiver *receiver) {
    if (pipeNo == 0 || pipeNo > RADIO_MAX_PIPES) {
        DEBUG("Wrong pipe number: %d", (int) pipeNo);
    }
    else {
        _receivers[pipeNo - 1] = receiver;
        _radio->openReadingPipe(pipeNo, RADIO_BEDROOM_ADDR);
    }
}

MyReceiver *MyMulticeiver::get(uint8_t pipeNo) {
    return _receivers[pipeNo - 1];
}
