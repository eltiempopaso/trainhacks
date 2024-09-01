#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef struct {
  int pin;
  boolean state;
} Command;

typedef struct {
  int pin;
  int remotePin;
  boolean state;
} Button;


const byte destinationAddress[6] = "00001";     
const int SLEEP_MS = 100;
const int PRINT_STATUS_TIMEOUT_MS = 30000;

RF24 radio(9, 10); 

Button buttons[] = { {3, 3, 0}, {4, 4, 0} };
int numButtons = sizeof(buttons) / sizeof(buttons[0]);

int msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
char buffer[50];

int select(int arr[], int n, int k) {
    if (n == 1) {
        return arr[0];
    }

    int pivot = arr[n / 2];

    int lows[n], highs[n], pivots[n];
    int lowCount = 0, highCount = 0, pivotCount = 0;

    for (int i = 0; i < n; i++) {
        if (arr[i] < pivot) {
            lows[lowCount++] = arr[i];
        } else if (arr[i] > pivot) {
            highs[highCount++] = arr[i];
        } else {
            pivots[pivotCount++] = arr[i];
        }
    }

    if (k < lowCount) {
        return select(lows, lowCount, k);
    } else if (k < lowCount + pivotCount) {
        return pivots[0];
    } else {
        return select(highs, highCount, k - lowCount - pivotCount);
    }
}

float torben_mogensen_median(int arr[], int n) {
    if (n % 2 == 1) {
        return select(arr, n, n / 2);
    } else {
        return 0.5 * (select(arr, n, n / 2 - 1) + select(arr, n, n / 2));
    }
}

void setup() 
{
  Serial.begin(9600);

  pinMode(buttons[0].pin, INPUT);
 
  radio.begin();                 
  radio.openWritingPipe(destinationAddress);
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening();  

  Serial.println("Inicialitzat!");
}

void loop()
{
  for (int nButton = 0; nButton < numButtons; nButton++)
  {
    int tmpStates[7];
    for (int i =0; i < 7; i++)
    {
      tmpStates[i] = digitalRead(buttons[nButton].pin);
    }
    int state = torben_mogensen_median(tmpStates, 7);
  
    if(state != buttons[nButton].state)
    {
      Command newCommand {buttons[nButton].remotePin, state};               
      bool ok = radio.write(&newCommand, sizeof(newCommand)); 

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

  delay(SLEEP_MS);

  msForNextStatusPrint = (msForNextStatusPrint > SLEEP_MS)? (msForNextStatusPrint-SLEEP_MS):0;

  if (msForNextStatusPrint == 0)
  {
    Serial.println("CPU OK. Estat actual:");

    for (int nButton = 0; nButton < numButtons; nButton++)
    {
      snprintf(buffer, sizeof(buffer)," * Boto local %d, rele remot %d, estat %s.", buttons[nButton].pin, buttons[nButton].remotePin, buttons[nButton].state==HIGH? "TANCAT":"OBERT");
      Serial.println(buffer);
    }

    msForNextStatusPrint = PRINT_STATUS_TIMEOUT_MS;
  }
}