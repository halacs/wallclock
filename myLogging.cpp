#include "myConfig.h"
#include "myLogging.h"
#include <SimpleSyslog.h> // https://github.com/scottchiefbaker/Arduino-SimpleSyslog

SimpleSyslog* syslog;

extern Config config;

void logging(int severity, const char* format, va_list args) {
  char* message;
  vasprintf(&message, format, args);
  
  if (syslog == 0) {
    if (strlen(config.syslog_server) > 1) {
      syslog = new SimpleSyslog(getAPName(), APPLICATION_NAME, config.syslog_server);
    }
  }

  char* severityMsg;
  switch(severity) {
    case PRI_ERROR:
      severityMsg = "ERROR";
      break;
    case PRI_WARNING:
      severityMsg = "WARNING";
      break;
    case PRI_INFO:
      severityMsg = "INFO";
      break;
    case PRI_DEBUG:
      severityMsg = "DEBUG";
      break;
    default:
      severityMsg = "UNKNOWN";
  }

  Serial.printf("[%s] %s\n", severityMsg, message);

  if (syslog != 0) {
    syslog->printf(FAC_LOCAL7, severity, message);
  }

  free(message);
}

void debugLog(const char* format, ...) {
  va_list args;
  va_start(args, format);

  logging(PRI_DEBUG, format, args);

  va_end(args);
}

void infoLog(const char* format, ...) {
  va_list args;
  va_start(args, format);

  logging(PRI_INFO, format, args);

  va_end(args);
}

void warningLog(const char* format, ...) {
  va_list args;
  va_start(args, format);

  logging(PRI_WARNING, format, args);

  va_end(args);
}

void errorLog(const char* format, ...) {
  va_list args;
  va_start(args, format);

  logging(PRI_ERROR, format, args);

  va_end(args);
}