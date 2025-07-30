#pragma once

typedef struct {
  uint16_t node;
} InitializationRequest;

typedef struct {
  byte pin;
  byte value;
} PinRequest;

// ALL NODES ADDRESSES
const uint16_t NODE_ID_ENTRADA  = 00;   
const uint16_t NODE_ID_ESTACIO  = 01; 

const uint16_t NODE_ID_MODUL_CENTRAL = 50;
const uint16_t NODE_ID_MODUL_ESCLAU  = 51;
