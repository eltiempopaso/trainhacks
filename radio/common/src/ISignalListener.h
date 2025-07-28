#pragma once

class ISignalListener {
    public:

    virtual void signalReceived ( const int code, const int value ) = 0;
};