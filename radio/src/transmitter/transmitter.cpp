#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include "common/protocol.h"
#include "transmitter.h"
#include <TaskScheduler.h>

#ifndef NRF24_NETWORK_CHANNEL
#define NRF24_NETWORK_CHANNEL 91
#endif

// USER DECLARED
bool checkIfThereAreChanges();
void initHw();
void readInputs(); // this task should be moved to user side. 

const int ledPin = 5;
typedef enum {
  OK,
  ERROR,
  NO_REMOTE
} GeneralStatus;

GeneralStatus generalStatus = OK;

// Create a scheduler object
Scheduler runner;

// Task functions
void sendPinRequests();
void nrf24Network();
void statusLed();

// Define tasks
Task taskSendPinRequests   (300, TASK_FOREVER, &sendPinRequests); 
Task taskNrf24Network (100, TASK_FOREVER, &nrf24Network); 
Task taskStatusLED (500, TASK_FOREVER, &statusLed); 
Task taskReadPinInputs (30, TASK_FOREVER, &readInputs); 

RF24 radio(10, 9); 
RF24Network network(radio);  // Network uses that radio


void setup()  {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // switch on led while initializing
			      //
  Serial.begin(9600);
  while (!Serial) { } // some boards need this because of native USB capability

  if (!radio.begin()) {
    logMessage("Radio hardware not responding!");
    while (1) {
      // hold in infinite loop
    }
  }
  
  radio.setChannel(NRF24_NETWORK_CHANNEL);
  network.begin(/*node address*/ THIS_NODE);

  logMessage("network initialized. This node id %d.", THIS_NODE);

  initHw();

  runner.addTask(taskSendPinRequests);
  runner.addTask(taskNrf24Network);
  runner.addTask(taskStatusLED);
  runner.addTask(taskReadPinInputs);

  taskSendPinRequests.enable();
  taskNrf24Network.enable();
  taskStatusLED.enable();
  taskReadPinInputs.enable();

  logMessage("Antena emisora arrencada!");
}

void loop() {
  runner.execute();
}

bool receiveMessages() {
  bool initializationRequested = false;
  InitializationRequest ir;

  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;  // If so, grab it and print it out
    uint16_t size = network.read(header, &ir, sizeof(ir));

    initializationRequested = true; // assuming I can only receive initialization messages
    logMessage("Peticio SYNC rebuda - %d.", size);
  }

  return initializationRequested;
}

bool thereAreChanges = true;
void sendPinRequests() {
  const bool neededInitialization = receiveMessages();
  const bool changesDetected = checkIfThereAreChanges();
  
  if (!thereAreChanges) thereAreChanges = changesDetected;

  if (thereAreChanges || neededInitialization) {
    RF24NetworkHeader header(/*to node*/ RECEIVER_NODE);            
    bool ok = network.write(header, requests, requestsSize);

    if (ok) {
      logMessage("SYNC OK");

      thereAreChanges = false; 

      generalStatus = OK;
    } else {
      // keep thereAreChanges = true to resend message
      logMessage("SYNC ERROR. Node ESTACIO no trobat %d. Sent size %d.", RECEIVER_NODE, requestsSize);

      generalStatus = ERROR;
    }
  }
}

void nrf24Network() {
  network.update();  // Check the network regularly
}

void statusLed() {
  static int counter = 0;
  static int LED_CURRENT = LOW;

  counter = (counter+1)%4;

  if (generalStatus == OK && counter ==0) { // blink slowly
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    logMessage("Alive. OK. TOOGLE");
  } else if (generalStatus == ERROR) {
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    logMessage("Alive. CONN ERROR. TOOGLE");
  }
}
