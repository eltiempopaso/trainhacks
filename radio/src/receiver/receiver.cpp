#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include <PCA95x5.h>

#include "common/protocol.h"
#include "receiver.h"


void initHw();
void onRequestReceived(const PinRequest & aRequest, const bool initialized);

RF24 radio(9, 10);
RF24Network network(radio);  // Network uses that radio

const int ledPin = 5;

bool initialized = false;

// Create a scheduler object
Scheduler runner;

void printStatus();
void receiveMessages();
void nrf24Network();

Task taskReceiveMessages(90, TASK_FOREVER, &receiveMessages); 
Task taskPrintStatus (5000, TASK_FOREVER, &printStatus); 
Task taskNrf24Network (100, TASK_FOREVER, &nrf24Network);

void setup()  {
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  if (!radio.begin()) {
    Serial.println(F("ERROR GREU. El NRF24 no respon!"));
    while (1) {
      // hold in infinite loop
    }
  }

  radio.setChannel(90);
  network.begin(/*node address*/ THIS_NODE);

  initHw();  

  printStatus(); // force sending initialization

  runner.addTask(taskPrintStatus);
  runner.addTask(taskReceiveMessages);
  runner.addTask(taskNrf24Network);

  taskPrintStatus.enable();
  taskReceiveMessages.enable();
  taskNrf24Network.enable();
  //Serial.println("Antena configurada OK.");
}

void loop() {
   // Execute scheduled tasks
  runner.execute();
}

void printStatus() {
  logMessage("Estat actual: %s", initialized? "INICIALITZAT": "SENSE INICIALITZAR");
  
  if (!initialized) {
    InitializationRequest ir { THIS_NODE };
    RF24NetworkHeader header(/*to node*/ EMITTER_NODE);            
    bool ok = network.write(header, &ir, sizeof(ir));

    logMessage("Peticio SYNC enviada al node EMISOR.");
  }
}

void receiveMessages() {
  static byte receivingBuffer[MAX_PAYLOAD_SIZE];

  while (network.available()) {  // Is there anything ready for us?

    // assuming the only message i can receive is the pin request buffer

    RF24NetworkHeader header;  // If so, grab it and print it out
    uint16_t size = network.read(header, receivingBuffer, MAX_PAYLOAD_SIZE);

    int nElements = size/sizeof(PinRequest);

    logMessage("Missatge rebut. Estat %d reles:", nElements);

    for (int i = 0; i<nElements; i++) {
      const PinRequest & aRequest = ((PinRequest *)receivingBuffer)[i];
      onRequestReceived( aRequest, initialized );

      logMessage("Configuro Pin %d. Estat %s.", aRequest.pin, aRequest.value==HIGH? "TANCAT": "OBERT");
    }

    initialized = true;
  }
}

void nrf24Network() {
  network.update();  // Check the network regularly
}
