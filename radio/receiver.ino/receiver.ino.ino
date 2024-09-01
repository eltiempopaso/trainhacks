#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef struct {
  int pin;
  boolean state;
} Command;

typedef struct {
  int pinid;
  int currentState;
} OutputMechanism;

const byte myAddress[6] = "00001";
const int SLEEP_MS = 100;
const int PRINT_STATUS_TIMEOUT_MS = 30000;

RF24 radio(9, 10);

OutputMechanism outputs[] = { {3, LOW}, {4, LOW}};
int numOutputs = sizeof(outputs) / sizeof(outputs[0]);

int msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
char buffer[50];

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

  for (int i = 0; i < numOutputs; i++)
  {
    pinMode(outputs[i].pinid, OUTPUT);    

    snprintf(buffer, sizeof(buffer), "Configurant pint %d com sortida.", outputs[i].pinid);
    Serial.println(buffer);
  }
  
  radio.begin();
  radio.openReadingPipe(0, myAddress); 
  radio.setPALevel(RF24_PA_MIN);       
  radio.startListening();              

  Serial.println("Radio inicialitzada OK. Adreca 1.");
}

void loop()
{
  if (radio.available())            
  {
    Command aCommand;

    char text[32] = "";                 
    radio.read(&aCommand, sizeof(aCommand)); 

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

  delay(SLEEP_MS);

  msForNextStatusPrint = (msForNextStatusPrint > SLEEP_MS)? (msForNextStatusPrint-SLEEP_MS):0;
  
  if (msForNextStatusPrint == 0)
  {
    Serial.println("CPU OK. Estat actual:");

    msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
  }
}