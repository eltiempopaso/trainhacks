#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>


typedef struct {
  int pin;
  boolean state;
} Command;

typedef struct {
  int pinid;
  int currentState;
} OutputMechanism;

RF24 radio(9, 10);
RF24Network network(radio);  // Network uses that radio

const uint16_t THIS_NODE = 02;   // Address of our node in Octal format (04, 031, etc)
const uint16_t ENTRADA_NODE = 01;  // Address of the other node in Octal format

OutputMechanism outputs[] = { {3, LOW}, {4, LOW}};
int numOutputs = sizeof(outputs) / sizeof(outputs[0]);

char buffer[50];

// Create a scheduler object
Scheduler runner;

void printStatus();
void receive();
void nrf24Network();

Task taskReceive(100, TASK_FOREVER, &receive); 
Task taskPrintStatus (30000, TASK_FOREVER, &printStatus); 
Task taskNrf24Network (10, TASK_FOREVER, &nrf24Network); 

int findOutputPosition(int pinId) {
    for (int i = 0; i < numOutputs; i++) {
        if (outputs[i].pinid == pinId) {
            return i;  // Return the index if pinId matches
        }
    }
    return -1;  // Return -1 if pinId is not found
}

void setup() 
{
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  for (int i = 0; i < numOutputs; i++)
  {
    pinMode(outputs[i].pinid, OUTPUT);    

    snprintf(buffer, sizeof(buffer), "Configurant pint %d com sortida.", outputs[i].pinid);
    Serial.println(buffer);
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
  runner.addTask(taskReceive);
  runner.addTask(taskNrf24Network);

  Serial.println("Radio inicialitzada OK. Adreca 1.");
}

void loop()
{
   // Execute scheduled tasks
  runner.execute();
}

void printStatus()
{
  Serial.println("CPU OK. Estat actual:");
}

void receive()
{
  while (network.available()) {  // Is there anything ready for us?
 
    RF24NetworkHeader header;  // If so, grab it and print it out
    //payload_t payload;
    Command aCommand;
    network.read(header, &aCommand, sizeof(aCommand));
    
    int position = findOutputPosition (aCommand.pin);

    if (position != -1)
    {
      digitalWrite(aCommand.pin, aCommand.state);
      outputs[position].currentState = aCommand.state;

      snprintf(buffer, sizeof(buffer), "Rebuda peticio. Pin %d estat %s.", aCommand.pin, aCommand.state==HIGH? "TANCAT": "OBERT");
      Serial.println(buffer);
    }
    else
    {
      snprintf(buffer, sizeof(buffer), "ERROR. Rebuda peticio. Pin %d (INCORRECTE) estat %s.", aCommand.pin, aCommand.state==HIGH? "TANCAT": "OBERT");
      Serial.println(buffer);
    }
  }
}

void nrf24Network()
{
  network.update();  // Check the network regularly
}