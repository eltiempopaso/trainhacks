#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include <PCA95x5.h>

#include "protocol.h"

#include "src/Desvio/Desvio.h"

#define NRELES_EXPANDERS 3
PCA9535 relesExpander[NRELES_EXPANDERS];

typedef struct {
  int pinid;
  int currentState;
} OutputMechanism;

RF24 radio(9, 10);
RF24Network network(radio);  // Network uses that radio

const uint16_t THIS_NODE = NODE_ID_ESTACIO;
const uint16_t ENTRADA_NODE = NODE_ID_ENTRADA;

OutputMechanism outputs[] = { {3, LOW}, {4, LOW}};
int numOutputs = sizeof(outputs) / sizeof(outputs[0]);
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

int findOutputPosition(int pinId) {
    for (int i = 0; i < numOutputs; i++) {
        if (outputs[i].pinid == pinId) {
            return i;  // Return the index if pinId matches
        }
    }
    return -1;  // Return -1 if pinId is not found
}

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
#if 0
      int position = findOutputPosition (aRequest.pin);

      if (position != -1) {
        //INVERTING BECAUSE OF PULLUPS
        outputs[position].currentState = aRequest.value==HIGH?LOW:HIGH;

        digitalWrite(aRequest.pin, outputs[position].currentState);
        if (!initialized) // trying to avoid glinch
        {
          pinMode(aRequest.pin, OUTPUT);
          digitalWrite(aRequest.pin, outputs[position].currentState);

          snprintf(buffer, sizeof(buffer), "Configuro Pin %d com a sortida.", aRequest.pin);
          Serial.println(buffer);

        }

        snprintf(buffer, sizeof(buffer), "Configuro Pin %d. Estat %s.", aRequest.pin, aRequest.value==HIGH? "TANCAT": "OBERT");
        Serial.println(buffer);
      } else {
        snprintf(buffer, sizeof(buffer), "ERROR. Pin %d (INCORRECTE) estat %s.", aRequest.pin, aRequest.value==HIGH? "TANCAT": "OBERT");
        Serial.println(buffer);
      }
      #endif
    }

    initialized = true;
  }
}

void nrf24Network() {
  network.update();  // Check the network regularly
}
