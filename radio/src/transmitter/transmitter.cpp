#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include "common/protocol.h"
#include "transmitter.h"
#include <TaskScheduler.h>


// USER DECLARED
bool checkIfThereAreChanges();
void userInits();
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
void printStatus();
void sendPinRequests();
void nrf24Network();
void statusLed();

// Define tasks
Task taskPrintStatus(5000, TASK_FOREVER, &printStatus); 
Task taskSendPinRequests   (200, TASK_FOREVER, &sendPinRequests); 
Task taskNrf24Network (100, TASK_FOREVER, &nrf24Network); 
Task taskStatusLED (500, TASK_FOREVER, &statusLed); 
Task taskReadPinInputs (30, TASK_FOREVER, &readInputs); 

RF24 radio(9, 10); 
RF24Network network(radio);  // Network uses that radio


void setup()  {
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {
      // hold in infinite loop
    }
  }
  
  radio.setChannel(90);
  network.begin(/*node address*/ THIS_NODE);

  pinMode(ledPin, OUTPUT);

  runner.addTask(taskPrintStatus);
  runner.addTask(taskSendPinRequests);
  runner.addTask(taskNrf24Network);
  runner.addTask(taskStatusLED);
  runner.addTask(taskReadPinInputs);

  taskPrintStatus.enable();
  taskSendPinRequests.enable();
  taskNrf24Network.enable();
  taskStatusLED.enable();
  taskReadPinInputs.enable();

  userInits();

  Serial.println("Inicialitzat!");
}

void loop() {
  // Execute scheduled tasks
  runner.execute();
}

void printStatus() {
  Serial.println("CPU OK. Estat actual:");
	
  /*
  for (int nButton = 0; nButton < numButtons; nButton++) {
    logMessage(" * Boto local %d, rele remot %d, estat %s.", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
  }
    */
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
    
    logMessage("Peticio SYNC rebuda.");
  }

  return initializationRequested;
}

bool thereAreChanges = true;


void sendPinRequests() {
  bool neededInitialization = receiveMessages();

  bool changesDetected = checkIfThereAreChanges();
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
      logMessage("SYNC ERROR. Node ESTACIO no trobat.");

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
