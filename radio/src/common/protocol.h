#pragma once

typedef struct {
  uint16_t node;
} InitializationRequest;

typedef struct {
  byte pin;
  byte value;
} PinRequest;

// ALL NODES ADDRESSES

// XARXA TERRAT
const uint16_t NODE_ID_ENTRADA  = 00;   
const uint16_t NODE_ID_ESTACIO  = 01; 

// XARXA TERRAT
const uint16_t NODE_ID_MODUL_CENTRAL = 00;
const uint16_t NODE_ID_MODUL_ESCLAU  = 01;
