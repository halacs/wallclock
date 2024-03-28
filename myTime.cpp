#include "myTime.h"

#include <AceWire.h> // TwoWireInterface
#include <Wire.h> // TwoWire, Wire
#include <AceTimeClock.h> // https://github.com/bxparks/AceTimeClock/
#include "myConfig.h"

extern Config config;

using namespace ace_time;
using ace_time::acetime_t;
using ace_time::OffsetDateTime;
using ace_time::clock::SystemClockLoop;
using ace_time::clock::NtpClock;
using ace_time::clock::DS3231Clock;

using WireInterface = ace_wire::TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
DS3231Clock<WireInterface> dsClock(wireInterface);

//NtpClock ntpClock("time.kfki.hu");
//SystemClockLoop systemClock(&ntpClock /*reference*/, &dsClock /*backup*/);
NtpClock ntpClock;
SystemClockLoop* systemClock;

// Create an ExtendedZoneManager with the entire TZ Database of Zone and Link
// entries. Cache size of 3 means that it can support 3 concurrent timezones
// without performance penalties.
static const int CACHE_SIZE = 3;
static ExtendedZoneProcessorCache<CACHE_SIZE> zoneProcessorCache;
static ExtendedZoneManager manager(zonedbx::kZoneAndLinkRegistrySize, zonedbx::kZoneAndLinkRegistry, zoneProcessorCache);

void printCurrentTime() {
  acetime_t nowSeconds = systemClock->getNow();
  TimeZone currentTz = manager.createForZoneName(config.timezone);
  ZonedDateTime localTime = ZonedDateTime::forEpochSeconds(nowSeconds, currentTz);
  //localTime.printTo(Serial);
  //Serial.println();
}

Time getLocalTime() {
  acetime_t nowSeconds = systemClock->getNow();
  TimeZone currentTz = manager.createForZoneName(config.timezone);
  ZonedDateTime localTime = ZonedDateTime::forEpochSeconds(nowSeconds, currentTz);
  
  Time t;
  t.year = localTime.year();
  t.month = localTime.month();
  t.day = localTime.day();

  t.hour = localTime.hour();
  t.minute = localTime.minute();
  t.second = localTime.second();
  
  printCurrentTime(); // only for debug purposes

  return t;
}

void syncTimeSetup() {
  new (&ntpClock) NtpClock(config.ntp_server);
  systemClock = new SystemClockLoop(&ntpClock /*reference*/, &dsClock /*backup*/);

  Wire.begin();
  wireInterface.begin();

  dsClock.setup();

  //Serial.printf("NTP server: %s\n", ntpClock.getServer());
  ntpClock.setup();
  if (ntpClock.isSetup()) {
    infoLog("NTP setup OK.");
  } else {    
    errorLog("Connection failed... try again.");
  }

  systemClock->setup();
}

void syncTimeLoop() {
  systemClock->loop();
}
