#ifndef MY_CONFIG_H
#define MY_CONFIG_H
#include <Arduino.h>

#define ENABLE_OTA

// Our configuration structure.
//
// Never use a JsonDocument to store the configuration!
// A JsonDocument is *not* a permanent storage; it's only a temporary storage
// used during the serialization phase. See:
// https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/
struct Config {
  char syslog_server[64];
  char device_name[64];
  char ntp_server[64];
  char timezone[64];
  bool leading_zeros;
  bool clever;
  bool second_blinking;
  byte brightness;
  bool auto_brightness;
};

#define device_name_default "HalClock"
#define ntp_server_default "hu.pool.ntp.org"
#define timezone_default "Europe/Budapest"
#define syslog_server_default "192.168.0.6"
#define brightness_default 255
#define brightness_default_char "255"
#define second_blinking_default false
#define second_blinking_default_char "false"
#define clever_numbering_default true
#define clever_numbering_default_char "true"
#define leading_zeros_default true
#define leading_zeros_default_char "true"
#define auto_brightness_default true
#define auto_brightness_default_char "true"
#define auto_brightness_minimum_light 245

#define APPLICATION_NAME "HalClock"

/*

*/
#define SR_OE 15    // output enable (inverted)
/*
Data from the input serial shift register is placed in the output register
with a rising pulse on the storages resister clock (STCP).
*/
#define SR_RCLK 12   // storages resister clock (STCP)
/*
An eight bit shift register accpets data from the serial input (DS) on
each positive transition of the shift register clock (SHCP).
*/
#define SR_SRCLK 14 // shift register clock (SHCP)
/*

*/
#define SR_SER 13   // serial input (DS)

char* getAPName();
char* getAPPassword();

#endif