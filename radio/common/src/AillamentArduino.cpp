#include "AillamentArduino.h"

void AillamentArduino::signalReceived( const int code, const int value  ) {
    if (code == config_.signalCode) {
        digitalWrite(config_.pinid, value == 0? 1:0); // inversion due to pullups
    } else {
        // do nothing. signal is for another actuator
    }
}