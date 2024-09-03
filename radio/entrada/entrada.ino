#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#include <TaskScheduler.h>
#include "torbenmogensen.h"
#include "protocol.h"

typedef struct {
  int pin;
  int remotePin;
  int state;
  int lastReads[5];
  int last;
} Button;

RF24 radio(9, 10); 
RF24Network network(radio);  // Network uses that radio

const uint16_t THIS_NODE  = 00;   // Address of our node in Octal format
const uint16_t OTHER_NODE = 01;  // Address of the other node in Octal format

Button buttons[] = { {3, 3, 0}, {4, 4, 0} };
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

void setup() 
{
  Serial.begin(9600);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  pinMode(buttons[0].pin, INPUT);
  pinMode(ledPin, OUTPUT);
 
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

bool receiveMessages()
{
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

void readInputs()
{
  //Serial.println("llegint pins");

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

    int & index = buttons[nButton].last;
    index = (index+1)%5;
    buttons[nButton].lastReads[index] = state;
  }
}

void sendPinRequests()
{
  bool neededInitialization = receiveMessages();

  for (int nButton = 0; nButton < numButtons; nButton++)
  {
    int sum = 0;
    for (int i = 0; i < 5; i++)
    {
      sum+=buttons[nButton].lastReads[i];
    }
    
    if (sum == 0 && buttons[nButton].state == HIGH)
    {    
      thereAreChanges = true; 
      buttons[nButton].state = LOW;

      snprintf(buffer, sizeof(buffer), "CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);

    }
    else if (sum == 5 && buttons[nButton].state == LOW)
    {
      thereAreChanges = true;
      buttons[nButton].state = HIGH;

      snprintf(buffer, sizeof(buffer), "CANVI DETECTAT. Pin %d, remot %d. Estat: %s", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);

    }
    else
    {
      // sense canvis
    }
  }

  if (thereAreChanges || neededInitialization)
  {
    PinRequest requests[numButtons];
    for (int nButton = 0; nButton < numButtons; nButton++)
    {
      requests[nButton] = {(byte)buttons[nButton].remotePin, buttons[nButton].state==0?0:1};
    }
    
    RF24NetworkHeader header(/*to node*/ OTHER_NODE);            
    bool ok = network.write(header, requests, sizeof(requests));

    if (ok)
    {
      snprintf(buffer, sizeof(buffer), "SYNC OK");
      Serial.println(buffer);

      thereAreChanges = false; 

      generalStatus = OK;
    }
    else
    {
      // keep thereAreChanges = true to resend message
      snprintf(buffer, sizeof(buffer), "SYNC ERROR. Node ESTACIO no trobat.");
      Serial.println(buffer);

      generalStatus = ERROR;
    }
  }
}

void nrf24Network()
{
  //Serial.println("network update");
  network.update();  // Check the network regularly
}

void statusLed()
{
  static int counter = 0;
  static int LED_CURRENT = LOW;

  counter = (counter+1)%4;

  if (generalStatus == OK && counter ==0) // move slowly
  {
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    Serial.println("OK. TOOGLE");
  }
  else if (generalStatus == ERROR)
  {
    LED_CURRENT = LED_CURRENT==LOW?HIGH:LOW;
    digitalWrite(ledPin, LED_CURRENT);    

    Serial.println("ERROR. TOOGLE");
  }
}
