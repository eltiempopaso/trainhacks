#include "src/transmitter/transmitter.h"

#define NSAMPLES_CHANGES_FILTER 5

typedef struct {
  uint8_t nexpander;
  uint8_t pin;
  uint8_t remotePin;
  uint8_t state;
  uint8_t lastReads[NSAMPLES_CHANGES_FILTER];
  uint8_t last;
} Button;

const uint16_t THIS_NODE  = NODE_ID_MODUL_CENTRAL;
const uint16_t RECEIVER_NODE = NODE_ID_MODUL_ESCLAU;

Button buttons[] = { {3, 3, 0}, {4, 4, 0} };
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

PinRequest requests[numButtons] = {0};
const unsigned requestsSize = sizeof(requests);

void readInputs() {
  //Serial.println("llegint pins");

  for (int nButton = 0; nButton < numButtons; nButton++) {
    #ifdef USE_TORBEN_MOGENSEN
    int tmpStates[7];
    for (int i =0; i < 7; i++) {
      tmpStates[i] = digitalRead(buttons[nButton].pin);
    }
    int state = torben_mogensen_median(tmpStates, 7);
    #else
    int state = digitalRead(buttons[nButton].pin);
    #endif

    uint8_t & index = buttons[nButton].last;
    index = (index+1)%5;
    buttons[nButton].lastReads[index] = state;
  }
}


bool checkIfThereAreChanges() {
  bool thereAreChanges = false;

  for (int nButton = 0; nButton < numButtons; nButton++) {
    int sum = 0;
    for (int i = 0; i < NSAMPLES_CHANGES_FILTER; i++) {
      sum+=buttons[nButton].lastReads[i];
    }
    
    if (sum == 0 && buttons[nButton].state == HIGH) {    
      thereAreChanges = true; 
      buttons[nButton].state = LOW;

      logMessage("CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
    } else if (sum == NSAMPLES_CHANGES_FILTER && buttons[nButton].state == LOW) {
      thereAreChanges = true;
      buttons[nButton].state = HIGH;

      logMessage("CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
    } else {
      // sense canvis
    }
  }

  for (int nButton = 0; nButton < numButtons; nButton++) {
    requests[nButton] = {(byte)buttons[nButton].remotePin, buttons[nButton].state==0?0:1};
  }

  return thereAreChanges;
}

void userInits(/*Scheduler & runner*/) {

}
