#include "SelectorCorrent.h"

void SelectorCorrent::signalReceived( const int code, const int value  ) {
    if (code == pinConfig_.signalCode) { // move position 1

        //Potser vull usar algun rele mes per treure corrent?? Sino.. funciona igual que inversor

        expander_->write(pinConfig_.pinid0, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
        expander_->write(pinConfig_.pinid1, value == 0? PCA95x5::Level::L: PCA95x5::Level::H);
    } else {
        // do nothing. signal is for another actuator
    }
}