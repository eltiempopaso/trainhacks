#!/bin/bash


# SPECIFIC CONFIGS....

# Default NRF24_NETWORK_CHANNEL (can be overridden via env var)
NRF24_NETWORK_CHANNEL=${NRF24_NETWORK_CHANNEL:-90}


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

# Default board (can be overridden via env var)
FQBN=${FQBN:-"arduino:avr:nano:cpu=atmega328old"}
PORT=${PORT:-"/dev/ttyUSB1"}


# Default sketch path is ".."
SKETCH_PATH=".."
MODE="compile"

# Argument parsing
if [[ "$1" == "upload" || "$1" == "compile" ]]; then
  MODE="$1"
elif [[ -n "$1" ]]; then
  SKETCH_PATH="$1"
  [[ "$2" == "upload" || "$2" == "compile" ]] && MODE="$2"
fi

# Convert to absolute paths
SKETCH_PATH=$(abs_path "$SKETCH_PATH")
BUILD_PATH=$(abs_path ".")/output_build

# Create build path if it doesn't exist
mkdir -p "$BUILD_PATH"

echo "🔧 Board             : $FQBN"
echo "📁 Sketch            : $SKETCH_PATH"
echo "🛠️  Mode              : $MODE"
echo "📂 Build dir         : $BUILD_PATH"
echo "📡 NRF24 Network Chan: $NRF24_NETWORK_CHANNEL"
[[ "$MODE" == "upload" ]] && echo "🔌 Port              : $PORT"

# Compile
echo "🚧 Compiling sketch..."
arduino-cli compile \
  --fqbn "$FQBN" \
  --build-path "$BUILD_PATH" \
  --build-property "build.extra_flags='-DNRF24_NETWORK_CHANNEL=$NRF24_NETWORK_CHANNEL'" \
  "$SKETCH_PATH" || { echo "❌ Compilation failed."; exit 1; }

# Upload if requested
if [[ "$MODE" == "upload" ]]; then
  echo "📤 Uploading to board..."
  arduino-cli upload \
    --fqbn "$FQBN" \
    --port "$PORT" \
    --input-dir "$BUILD_PATH" \
    "$SKETCH_PATH" || { echo "❌ Upload failed."; exit 1; }
  echo "✅ Upload completed."
else
  echo "✅ Compilation completed (no upload)."
fi

