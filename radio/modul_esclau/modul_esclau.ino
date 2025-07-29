#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include <PCA95x5.h>

#include "protocol.h"

#include "src/Desvio.h"
#include "src/Aillament.h"
#include "src/Inversor.h"
#include "src/SelectorCorrent.h"

#define NRELES_EXPANDERS 3
PCA9535 relesExpander[NRELES_EXPANDERS];

Aillament aillaments[] {
    Aillament(&relesExpander[0], {0,  0}),
    Aillament(&relesExpander[0], {1,  1}),
    Aillament(&relesExpander[0], {2,  2}),
    Aillament(&relesExpander[0], {3,  3}),
    Aillament(&relesExpander[0], {4,  4}),
    Aillament(&relesExpander[0], {5,  5}),
    Aillament(&relesExpander[0], {6,  6}),
    Aillament(&relesExpander[0], {7,  7}),
    Aillament(&relesExpander[0], {8,  8}),
    Aillament(&relesExpander[0], {9,  9}),
    Aillament(&relesExpander[0], {10, 10}),
    Aillament(&relesExpander[0], {11, 11}),
    Aillament(&relesExpander[0], {12, 12}),
};
const int numAillaments = sizeof(aillaments) / sizeof(aillaments[0]);

Desvio desvios[] = {
    Desvio(&relesExpander[1], {13,0},  {14,1}),
    Desvio(&relesExpander[1], {15,2},  {16,3}),
    Desvio(&relesExpander[1], {17,4},  {18,5}),
    Desvio(&relesExpander[1], {19,6},  {20,7}),
    Desvio(&relesExpander[1], {21,8},  {22,9}),
    Desvio(&relesExpander[1], {23,10}, {24,11}),
};
const int numDesvios = sizeof(desvios) / sizeof(desvios[0]);

Inversor inversors[] {
    Inversor(&relesExpander[3], {25,  0, 1}),
    Inversor(&relesExpander[3], {26,  2, 3}),
};
const int numInversors = sizeof(inversors) / sizeof(inversors[0]);

SelectorCorrent selectors[] {
    SelectorCorrent(&relesExpander[3], {27,  4, 5}), // 6 y 7 para apagar potencia....
};
const int numSelectors = sizeof(selectors) / sizeof(selectors[0]);


RF24 radio(9, 10);
RF24Network network(radio);  // Network uses that radio

const uint16_t THIS_NODE = NODE_ID_ESTACIO;
const uint16_t ENTRADA_NODE = NODE_ID_ENTRADA;

bool initialized = false;

char buffer[50];

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

  Wire.begin();
  for (int i = 0; i < NRELES_EXPANDERS; i++) {
    relesExpander[i].attach(Wire);
    relesExpander[i].polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    relesExpander[i].direction(PCA95x5::Direction::OUT_ALL);
    relesExpander[i].write(PCA95x5::Level::L_ALL);
  }
  

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
  snprintf(buffer, sizeof(buffer), "Estat actual: %s", initialized? "INICIALITZAT": "SENSE INICIALITZAR");
  Serial.println(buffer);

  if (!initialized) {
    InitializationRequest ir { THIS_NODE };
    RF24NetworkHeader header(/*to node*/ ENTRADA_NODE);            
    bool ok = network.write(header, &ir, sizeof(ir));

   // same algorithm if destination received or not (write ok true or false)

    snprintf(buffer, sizeof(buffer), "Peticio SYNC enviada al node ENTRADA.");
    Serial.println(buffer);
  }
}

void receiveMessages() {
  static byte receivingBuffer[MAX_PAYLOAD_SIZE];

  while (network.available()) {  // Is there anything ready for us?

    // assuming the only message i can receive is the pin request buffer

    RF24NetworkHeader header;  // If so, grab it and print it out
    uint16_t size = network.read(header, receivingBuffer, MAX_PAYLOAD_SIZE);

    int nElements = size/sizeof(PinRequest);

    snprintf(buffer, sizeof(buffer), "Missatge rebut. Estat %d reles:", nElements);
    Serial.println(buffer);

    for (int i = 0; i<nElements; i++) {
      const PinRequest & aRequest = ((PinRequest *)receivingBuffer)[i];

      for (int n = 0; n < numAillaments; n++) {
        aillaments[n].signalReceived(aRequest.pin, aRequest.value); // should i invert because of pullups?  
      }

      for (int n = 0; n < numDesvios; n++) {
        desvios[n].signalReceived(aRequest.pin, aRequest.value); // should i invert because of pullups?  
      }

      for (int n = 0; n < numInversors; n++) {
        inversors[n].signalReceived(aRequest.pin, aRequest.value); // should i invert because of pullups?  
      }     

      for (int n = 0; n < numSelectors; n++) {
        selectors[n].signalReceived(aRequest.pin, aRequest.value); // should i invert because of pullups?  
      }     

      snprintf(buffer, sizeof(buffer), "Configuro Pin %d. Estat %s.", aRequest.pin, aRequest.value==HIGH? "TANCAT": "OBERT");
      Serial.println(buffer);
    }

    initialized = true;
  }
}

void nrf24Network() {
  network.update();  // Check the network regularly
}
