#pragma once

#include "common/protocol.h"
#include "common/logger.h"

#include "actuators/Desvio.h"
#include "actuators/Aillament.h"
#include "actuators/Inversor.h"
#include "actuators/SelectorCorrent.h"

void setup();

void loop();

extern const uint16_t THIS_NODE;
extern const uint16_t EMITTER_NODE;



