#pragma once

#include "ISignalListener.h"
#include <PCA95x5.h>



class AillamentArduino : ISignalListener {

    public:

    typedef struct {
        int signalCode;
        int pinid;
    } Config;
    
    explicit AillamentArduino( Config config ) : config_(config) {}

    void signalReceived( const int code, const int value );

    private:

    Config config_;
};