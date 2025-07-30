#!/bin/bash

# Enable alias expansion
shopt -s expand_aliases
source ~/.bash_aliases

# Ensure realpath or fallback to readlink
abs_path() {
  if command -v realpath >/dev/null 2>&1; then
    realpath "$1"
  else
    readlink -f "$1"
  fi
}

# Default values (can be overridden)
FQBN=${FQBN:-"arduino:avr:nano:cpu=atmega328old"}

# Sketch path (positional arg or default to "..")
RAW_SKETCH_PATH=${1:-".."}

# Convert to absolute paths
SKETCH_PATH=$(abs_path "$RAW_SKETCH_PATH")
BUILD_PATH=$(abs_path ".")/output_build

# Create build path if it doesn't exist
mkdir -p "$BUILD_PATH"

# Compile
arduino-cli compile \
  --fqbn "$FQBN" \
  --build-path "$BUILD_PATH" \
  "$SKETCH_PATH"
