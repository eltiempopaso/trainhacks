#include "Desvio.h"

void Desvio::signalReceived( const int code, const int value  ) {
    if (code == dir0_.signalCode) { // move position 1
        expander_->write(dir0_.pinid, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
    } else if (code == dir1_.signalCode) { // move position 2
        expander_->write(dir1_.pinid, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
    } else {
        // do nothing. signal is for another actuator
    }
}