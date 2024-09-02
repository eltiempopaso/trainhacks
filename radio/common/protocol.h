#pragma once

typedef struct {
  uint16_t node;
} InitializationRequest;

typedef struct {
  byte pin;
  byte value;
} PinRequest;
