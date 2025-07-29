#pragma once

#include "ISignalListener.h"
#include <PCA95x5.h>

class Desvio : ISignalListener {

    public:

    typedef struct {
        int signalCode;
        int pinid;
    } PinPair;
    
    explicit Desvio( PCA9535 * expander, PinPair dir0, PinPair dir1 ) 
                                        : expander_(expander), dir0_(dir0), dir1_(dir1) {}

    void signalReceived( const int code, const int value );

    private:

    PCA9535 * expander_;
    PinPair dir0_;
    PinPair dir1_;

};