#include "Inversor.h"

void Inversor::signalReceived( const int code, const int value  ) {
    if (code == pinConfig_.signalCode) { // move position 1
        expander_->write(pinConfig_.pinid0, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
        expander_->write(pinConfig_.pinid1, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
    } else {
        // do nothing. signal is for another actuator
    }
}