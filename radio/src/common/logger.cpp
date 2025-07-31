#include "logger.h"

void logMessage(const char* format, ...) {
  char buffer[128];  // Adjust buffer size as needed
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.println(buffer);
}
