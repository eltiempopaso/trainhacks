#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include <Adafruit_PCF8574.h>
#include "protocol.h"

typedef struct {
  int nexpander;
  int pin;
  int remotePin;
  int state;
  int lastReads[5];
  int last;
} Button;

RF24 radio(9, 10); 
RF24Network network(radio);  // Network uses that radio

const uint16_t THIS_NODE  = NODE_ID_MODUL_CENTRAL;
const uint16_t OTHER_NODE = NODE_ID_MODUL_ESCLAU;

#define NUM_EXPANDERS 5
Adafruit_PCF8574 pcf[NUM_EXPANDERS];
uint8_t i2cAddrs[NUM_EXPANDERS] = { 0x20, 0x21, 0x22, 0x23, 0x24 };

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

int numButtons = sizeof(buttons) / sizeof(buttons[0]);

const int ledPin = 5;
typedef enum {
  OK,
  ERROR,
  NO_REMOTE
} GeneralStatus;

GeneralStatus generalStatus = OK;

//int msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
char buffer[50];

// Create a scheduler object
Scheduler runner;

// Task functions
void printStatus();
void readInputs();
void sendPinRequests();
void nrf24Network();
void statusLed();

// Define tasks
Task taskPrintStatus(5000, TASK_FOREVER, &printStatus); 
Task taskReadPinInputs (30, TASK_FOREVER, &readInputs); 
Task taskSendPinRequests   (200, TASK_FOREVER, &sendPinRequests); 
Task taskNrf24Network (100, TASK_FOREVER, &nrf24Network); 
Task taskStatusLED (500, TASK_FOREVER, &statusLed); 

void setup()  {
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  for (int i = 0; i < NUM_EXPANDERS; i++) {
  if (!pcf[i].begin(i2cAddrs[i], &Wire)) {
    Serial.println("Couldn't find PCF8574");
    while (1);
  }

  for (uint8_t p=0; p<8; p++) {
    pcf[i].pinMode(p, INPUT_PULLUP);
  }
  }
  

  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {
      // hold in infinite loop
    }
  }
  
  radio.setChannel(90);
  network.begin(/*node address*/ THIS_NODE);

  runner.addTask(taskPrintStatus);
  runner.addTask(taskReadPinInputs);
  runner.addTask(taskSendPinRequests);
  runner.addTask(taskNrf24Network);
  runner.addTask(taskStatusLED);

  taskPrintStatus.enable();
  taskSendPinRequests.enable();
  taskNrf24Network.enable();
  taskReadPinInputs.enable();
  taskStatusLED.enable();

  Serial.println("Inicialitzat!");
}

void loop() {
  // Execute scheduled tasks
  runner.execute();
}

void printStatus() {
  Serial.println("CPU OK. Estat actual:");

  for (int nButton = 0; nButton < numButtons; nButton++) {
    snprintf(buffer, sizeof(buffer)," * Boto local %d, rele remot %d, estat %s.", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
    Serial.println(buffer);
  }

}

bool receiveMessages() {
   //Serial.println("Rebent missatges");

  bool initializationRequested = false;
  InitializationRequest ir;

  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;  // If so, grab it and print it out
    //payload_t payload;
    
    uint16_t size = network.read(header, &ir, sizeof(ir));

    initializationRequested = true; // assuming the only message I can receive is the initialization one

    snprintf(buffer, sizeof(buffer), "Peticio SYNC rebuda.");
    Serial.println(buffer);
  }

  return initializationRequested;
}

bool thereAreChanges = true;
#define NREADS_EE_NOISE 1 // Filtering posible ee noise?
uint8_t expanderBytes[NUM_EXPANDERS][NREADS_EE_NOISE] = {0};

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
      const int tmp = (expanderBytes[buttons[nButton].nexpander][nreads] >> buttons[nButton].remotePin) & 1;
      if (tmp == 1) ones++;
      else zeros++;
    }
    const int state = ones > zeros? 1: 0;

    int & index = buttons[nButton].last;
    index = (index+1)%5;
    buttons[nButton].lastReads[index] = state;
  }  
}

void sendPinRequests() {
  bool neededInitialization = receiveMessages();

  for (int nButton = 0; nButton < numButtons; nButton++) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
      sum+=buttons[nButton].lastReads[i];
    }
    
    if (sum == 0 && buttons[nButton].state == HIGH) {    
      thereAreChanges = true; 
      buttons[nButton].state = LOW;

      snprintf(buffer, sizeof(buffer), "CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);
    } else if (sum == 5 && buttons[nButton].state == LOW) {
      thereAreChanges = true;
      buttons[nButton].state = HIGH;

      snprintf(buffer, sizeof(buffer), "CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);
    } else {
      // sense canvis
    }
  }

  if (thereAreChanges || neededInitialization) {
    PinRequest requests[numButtons];
    for (int nButton = 0; nButton < numButtons; nButton++) {
      requests[nButton] = {(byte)buttons[nButton].remotePin, buttons[nButton].state==0?0:1};
    }
    
    RF24NetworkHeader header(/*to node*/ OTHER_NODE);            
    bool ok = network.write(header, requests, sizeof(requests));

    if (ok) {
      snprintf(buffer, sizeof(buffer), "SYNC OK");
      Serial.println(buffer);

      thereAreChanges = false; 

      generalStatus = OK;
    } else {
      // keep thereAreChanges = true to resend message
      snprintf(buffer, sizeof(buffer), "SYNC ERROR. Node ESTACIO no trobat.");
      Serial.println(buffer);

      generalStatus = ERROR;
    }
  }
}

void nrf24Network() {
  //Serial.println("network update");
  network.update();  // Check the network regularly
}

void statusLed() {
  static int counter = 0;
  static int LED_CURRENT = LOW;

  counter = (counter+1)%4;

  if (generalStatus == OK && counter ==0) { // blink slowly
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    Serial.println("OK. TOOGLE");
  } else if (generalStatus == ERROR) {
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    Serial.println("ERROR. TOOGLE");
  }
}
