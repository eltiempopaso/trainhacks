#pragma once

#include "ISignalListener.h"
#include <PCA95x5.h>

class Inversor : ISignalListener {

    public:

    typedef struct {
        int signalCode;
        int pinid0;
        int pinid1;
    } PinConfig;
    
    explicit Inversor( PCA9535 * expander, PinConfig pinConfig ) 
                                        : expander_(expander), pinConfig_(pinConfig) {}

    void signalReceived( const int code, const int value );

    private:

    PCA9535 * expander_;
    PinConfig pinConfig_;
};