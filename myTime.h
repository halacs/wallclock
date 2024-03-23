#ifndef MY_TIME_H
#define MY_TIME_H
#include <Arduino.h>
#include "myLogging.h"

struct Time {
  int year;
  int month;
  int day;

  int hour;
  int minute;
  int second;
};

void syncTimeSetup();
void syncTimeLoop();
Time getLocalTime();

#endif