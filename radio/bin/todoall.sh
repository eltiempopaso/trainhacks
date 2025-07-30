#!/bin/bash

# Get absolute path to this script
SCRIPT_PATH="$(realpath "$0")"
ROOT_PATH="$(dirname "$SCRIPT_PATH")/../nodes"

# Find all directories named "bin" recursively from ROOT_PATH
find "$ROOT_PATH" -type d -name bin | while read -r BIN_DIR; do
  BUILD_SCRIPT="$BIN_DIR/build.sh"

  if [[ -x "$BUILD_SCRIPT" ]]; then
    echo "🔧 Running build.sh in: $BIN_DIR"
    (cd "$BIN_DIR" && ./build.sh)
  elif [[ -f "$BUILD_SCRIPT" ]]; then
    echo "⚠️  Found build.sh in $BIN_DIR but it's not executable. Skipping."
  else
    echo "ℹ️  No build.sh found in $BIN_DIR. Skipping."
  fi
done
