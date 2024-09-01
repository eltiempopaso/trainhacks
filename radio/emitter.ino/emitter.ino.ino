#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include "torbenmogensen.h"

typedef struct {
  int pin;
  boolean state;
} Command;

typedef struct {
  int pin;
  int remotePin;
  boolean state;
} Button;


//const byte destinationAddress[6] = "00001";     
//const int SLEEP_MS = 100;
const int PRINT_STATUS_TIMEOUT_MS = 30000;

RF24 radio(9, 10); 
RF24Network network(radio);  // Network uses that radio


const uint16_t THIS_NODE  = 01;   // Address of our node in Octal format
const uint16_t OTHER_NODE = 02;  // Address of the other node in Octal format


Button buttons[] = { {3, 3, 0}, {4, 4, 0} };
int numButtons = sizeof(buttons) / sizeof(buttons[0]);

//int msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
char buffer[50];

// Create a scheduler object
Scheduler runner;

// Task functions
void printStatus();
void readAndSendInputs();
void nrf24Network();

// Define tasks
Task taskPrintStatus(PRINT_STATUS_TIMEOUT_MS, TASK_FOREVER, &printStatus); 
Task taskReadAndSendInputs (100, TASK_FOREVER, &readAndSendInputs); 
Task taskNrf24Network (10, TASK_FOREVER, &nrf24Network); 

void setup() 
{
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  pinMode(buttons[0].pin, INPUT);
 
  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {
      // hold in infinite loop
    }
  }
  /*
  radio.begin();                 
  radio.openWritingPipe(destinationAddress);
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening();  
*/
  radio.setChannel(90);
  network.begin(/*node address*/ THIS_NODE);

  runner.addTask(taskPrintStatus);
  runner.addTask(taskReadAndSendInputs);
  runner.addTask(taskNrf24Network);

  Serial.println("Inicialitzat!");
}

void loop()
{
  // Execute scheduled tasks
  runner.execute();
}

void printStatus()
{
  Serial.println("CPU OK. Estat actual:");

    for (int nButton = 0; nButton < numButtons; nButton++)
    {
      snprintf(buffer, sizeof(buffer)," * Boto local %d, rele remot %d, estat %s.", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);
    }
}

void readAndSendInputs()
{
  for (int nButton = 0; nButton < numButtons; nButton++)
  {
    #ifdef USE_TORBEN_MOGENSEN
    int tmpStates[7];
    for (int i =0; i < 7; i++)
    {
      tmpStates[i] = digitalRead(buttons[nButton].pin);
    }
    int state = torben_mogensen_median(tmpStates, 7);
    #else
    int state = digitalRead(buttons[nButton].pin);
    #endif
  
    if(state != buttons[nButton].state)
    {
      Command newCommand {buttons[nButton].remotePin, state};   
      RF24NetworkHeader header(/*to node*/ OTHER_NODE);            
      bool ok = network.write(header, &newCommand, sizeof(newCommand));

      if (ok)
      {
        snprintf(buffer, sizeof(buffer), "CANVI OK. Boto %d, rele remot %d. Nou estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, state==HIGH? "TANCAT":"OBERT");
        Serial.println(buffer);
      }
      else
      {
        snprintf(buffer, sizeof(buffer), "CANVI ERROR. Node remot no respon. Fallo al enviar boto %d, rele remot %d. Nou estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, state==HIGH? "TANCAT":"OBERT");
        Serial.println(buffer);
      }

      buttons[nButton].state = state;      
    }
  }
}

void nrf24Network()
{
  network.update();  // Check the network regularly
/*
  payload_t payload = { millis(), packets_sent++ };
  RF24NetworkHeader header(/@to node@/ other_node);
  bool ok = network.write(header, &payload, sizeof(payload));
*/
}