#include <Adafruit_PCF8574.h>
#include "src/transmitter/transmitter.h"

#define NSAMPLES_CHANGES_FILTER 3

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

#define NUM_EXPANDERS 5
Adafruit_PCF8574 pcf[NUM_EXPANDERS];
uint8_t i2cAddrs[NUM_EXPANDERS] = { 0x20, 0x21, 0x22, 0x23, 0x24 };

#define NREADS_EE_NOISE 1 // Filtering posible ee noise?
uint8_t expanderBytes[NUM_EXPANDERS][NREADS_EE_NOISE] = {0};

Button buttons[] = { 
                      // AILLAMENT VIES
                     {0, 0, 0, 0},  
                     {0, 1, 1, 0},
                     {0, 2, 2, 0},
                     {0, 3, 3, 0},
                     {0, 4, 4, 0},
                     {0, 5, 5, 0},
                     {0, 6, 6, 0},
                     {0, 7, 7, 0},
                     {1, 0, 8, 0},
                     {1, 1, 9, 0},
                     {1, 2, 10, 0},
                     {1, 3, 11, 0},
                     {1, 4, 12, 0},

                      // DESVIOS
                     {2, 0, 13, 0}, 
                     {2, 1, 14, 0},

                     {2, 2, 15, 0},
                     {2, 3, 16, 0},

                     {2, 4, 17, 0},
                     {2, 5, 18, 0},

                     {2, 6, 19, 0},
                     {2, 7, 20, 0},

                     {3, 0, 21, 0},
                     {3, 1, 22, 0},

                     {3, 2, 23, 0},
                     {3, 3, 24, 0},

                     {3, 4, 25, 0},
                     {3, 5, 26, 0},
                      // INVERSOR GENERAL
                     {4, 0, 27, 0},
                      // INVERSOR LOCAL
                     {4, 1, 28, 0},
                      // SELECCIONADOR TRANSFORMADOR
                     {4, 2, 29, 0},
                    };
const int numButtons = sizeof(buttons) / sizeof(buttons[0]);


PinRequest requests[numButtons] = {0};
const unsigned requestsSize = sizeof(requests);

void readInputs() {
  //Serial.println("llegint pins");
  for (int i = 0; i < NUM_EXPANDERS; i++) {
    for (int nreads =0; nreads < NREADS_EE_NOISE; nreads++) {
      expanderBytes[i][nreads] = pcf[i].digitalReadByte();
    }
  }

  for (int nButton = 0; nButton < numButtons; nButton++) {
    int ones = 0, zeros = 0;
    for (int nreads =0; nreads < NREADS_EE_NOISE; nreads++) {
      const int tmp = (expanderBytes[buttons[nButton].nexpander][nreads] >> buttons[nButton].pin) & 1;
      if (tmp == 1) ones++;
      else zeros++;
    }
    const int state = ones > zeros? 1: 0;

    uint8_t & index = buttons[nButton].last;
    index = (index+1)%NSAMPLES_CHANGES_FILTER;
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
  Wire.begin();
  for (int i = 0; i < NUM_EXPANDERS; i++) {
	  if (!pcf[i].begin(i2cAddrs[i], &Wire)) {
	    logMessage("ERROR. Couldn't find PCF8574 with address %d", i2cAddrs[i]);
	    while (1);
	  }

	  for (uint8_t p=0; p<8; p++) {
	    pcf[i].pinMode(p, INPUT_PULLUP);
	  }
  }

}
