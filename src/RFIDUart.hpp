#pragma once

#include <Arduino.h>

class RFIDUart {
public:
    RFIDUart();
    void begin();
    void update();
    void send(const char* data);
    String receive();
    bool isAvailable();
    void clear();
    void end();
};