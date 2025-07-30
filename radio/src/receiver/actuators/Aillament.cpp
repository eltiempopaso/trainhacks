#include "Aillament.h"

void Aillament::signalReceived( const int code, const int value  ) {
    if (code == pinPair_.signalCode) { // move position 1
        expander_->write(pinPair_.pinid, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
    } else {
        // do nothing. signal is for another actuator
    }
}