#pragma once

#include "ISignalListener.h"
#include <PCA95x5.h>



class Aillament : ISignalListener {

    public:

    typedef struct {
        int signalCode;
        int pinid;
    } PinPair;
    
    explicit Aillament( PCA9535 * expander, PinPair pinPair ) 
                                        : expander_(expander), pinPair_(pinPair) {}

    void signalReceived( const int code, const int value );

    private:

    PCA9535 * expander_;
    PinPair pinPair_;
};