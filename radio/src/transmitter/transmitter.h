#pragma once

#include "common/protocol.h"
#include "common/logger.h"

void setup();

void loop();

extern const uint16_t THIS_NODE;
extern const uint16_t RECEIVER_NODE;

extern const unsigned requestsSize;
extern PinRequest requests[];



