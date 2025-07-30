#include "src/receiver/receiver.h"

const uint16_t THIS_NODE = NODE_ID_ESTACIO;
const uint16_t EMITTER_NODE = NODE_ID_ENTRADA;

typedef struct {
  int pinid;
  int currentState;
} OutputMechanism;


OutputMechanism outputs[] = { {3, LOW}, {4, LOW}};
int numOutputs = sizeof(outputs) / sizeof(outputs[0]);

int findOutputPosition(int pinId) {
    for (int i = 0; i < numOutputs; i++) {
        if (outputs[i].pinid == pinId) {
            return i;  // Return the index if pinId matches
        }
    }
    return -1;  // Return -1 if pinId is not found
}


void initHw() {
// DO NOTHING. THIS NODE INITS HW WHEN RECEIVING MESSAGE
}

void onRequestReceived( const PinRequest & aRequest, const bool initialized ) {

      int position = findOutputPosition (aRequest.pin);

      if (position != -1) {
        //INVERTING BECAUSE OF PULLUPS
        outputs[position].currentState = aRequest.value==HIGH?LOW:HIGH;

        digitalWrite(aRequest.pin, outputs[position].currentState);
        if (!initialized) // trying to avoid glinch
        {
          pinMode(aRequest.pin, OUTPUT);
          digitalWrite(aRequest.pin, outputs[position].currentState);
        }
      }
}
