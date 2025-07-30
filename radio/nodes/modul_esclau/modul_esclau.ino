#include "src/receiver/receiver.h"

const uint16_t THIS_NODE = NODE_ID_ESTACIO;
const uint16_t EMITTER_NODE = NODE_ID_ENTRADA;

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

void initHw() {
  Wire.begin();
  for (int i = 0; i < NRELES_EXPANDERS; i++) {
    relesExpander[i].attach(Wire);
    relesExpander[i].polarity(PCA95x5::Polarity::ORIGINAL_ALL);
    relesExpander[i].direction(PCA95x5::Direction::OUT_ALL);
    relesExpander[i].write(PCA95x5::Level::L_ALL);
  }
}

void onRequestReceived( const PinRequest & aRequest, const bool initialized ) {
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
}
