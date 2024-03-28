#include "myConfig.h"
#include <WiFiManager.h>

char* hostnameSingleton = 0;
char* getAPName() {
  if (hostnameSingleton == 0) {
    String hostString = String(WIFI_getChipId(),HEX);
    hostString.toUpperCase();
    String name = String("halclock") + hostString;

    hostnameSingleton = (char*)malloc(name.length() + 1);
    memcpy(hostnameSingleton, name.c_str(), name.length() + 1);
  }

  return hostnameSingleton;
}

char* getAPPassword() {
  return "password";
}