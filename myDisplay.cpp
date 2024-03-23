#include "myDisplay.h"
#include "myTime.h"
#include "myConfig.h"

extern Config config;

unsigned long displayMtime = 0;
void syncDisplayScheduler() {
  if(millis()-displayMtime >= 1000 ){
    updateDisplay();
    displayMtime = millis();
  }
}

void storeBit(byte bit) {
  if (bit == 1) {
    digitalWrite(SR_SER, HIGH);
  } else {
    digitalWrite(SR_SER, LOW);
  }

  digitalWrite(SR_SRCLK, HIGH);
  digitalWrite(SR_SRCLK, LOW);
}

void allDigitsToStorageRegisters() {
  digitalWrite(SR_RCLK, HIGH);
  digitalWrite(SR_RCLK, LOW);
}

void enableDisplay(bool enabled) {
  const int brightness_level = 255-config.brightness;

  if (enabled) {
    //digitalWrite(SR_OE, LOW);              // inverted
    analogWrite(SR_OE, brightness_level);  // 0..255 - The higher value the softer brightness
  } else {
    //digitalWrite(SR_OE, HIGH);             // inverted
    analogWrite(SR_OE, 255);                 // 0..255 - The higher value the softer brightness
  }
}

void convert(int n, byte &hi, byte &lo) {
  // Input validation: max 2 digits
  if (n > 99 || n < 0) {
    lo = 255;
    hi = 255;
    return;
  }

  lo = n % 10;
  hi = n / 10;
}

void sendByte(byte digitn, bool second, bool leadingDigit, bool clever) {
//  Serial.printf("%d --- ", digitn);

  byte data = 0;

  // map actual digit according to the physical wiring of the display
  if (digitn > 9 || digitn < 0) {
    // invalid input
    //Serial.print(" INVALID ");
    data = mapping[DIGIT_DASH];
  } else {
    if (clever) {
      data = mappingClever[digitn];  // map it a bit differently somewhere
    } else {
      data = mapping[digitn];       // map it in a standard way
    }
  }

  // remove leading zeros in case of need
  if (leadingDigit) {
    if (digitn == 0 && !config.leading_zeros) {
      data = mapping[DIGIT_OFF];  // turn off given digit to eliminate leading zero
    }
  }

  // deal with leds of second
  if (second) { // zeroing most significant bit if second led needs to be on. Second led controlled via digit4 and digit3 (two leds)
    const byte mask = 0b01111111;
    data = data & mask;
  }

  // send out data bit by bit in a serial manner (think of the four shift registers)
  //Serial.printf(" ; %x ;", data);
  int mask = 1;
  for (int i = 0; i < 8; i++) {
    byte bit = ((data & mask) == mask) ? 1 : 0;
    //Serial.printf("%d(%d) ", bit, mask);
    mask = mask << 1;
    storeBit(bit);
  }

  //Serial.println();
}

void show(byte digit1, byte digit2, byte digit3, byte digit4, bool second, bool clever) {
  //Serial.printf("Writing to display: %d%d:%d%d (SECOND %s)\n", digit1, digit2, digit3, digit4, second ? "ON" : "OFF");

  bool doItSmart1 = (clever && (digit1 == 1) && (digit2 != 1));
  bool doItSmart2 = (clever && (digit3 == 1) && (digit4 != 1));
  
  //Serial.printf("clever1: %d, clever2: %d\n", doItSmart1, doItSmart2);

  sendByte(digit1, second, true, doItSmart1);
  sendByte(digit2, second, false, false);
  sendByte(digit3, second, true, doItSmart2);
  sendByte(digit4, second, false, false);
  
  allDigitsToStorageRegisters();

  enableDisplay(true);  // TODO this might not the best place to keep the display on especially because of the brightness control (to be implemented)
}

void updateDisplay() {
  //Serial.println("update display");

  Time localTime = getLocalTime();
  Serial.printf("%d-%d-%d %d:%d:%d\n", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.minute, localTime.second);

  bool secondLedsOn = true;
  if (config.second_blinking) {
    secondLedsOn = localTime.second %2 == 0; // turn second leds on and off every second
  }

  bool clever = config.clever;
  byte hourHi, hourLo, minuteHi, minuteLo;
  convert(localTime.hour, hourHi, hourLo);
  convert(localTime.minute, minuteHi, minuteLo);
  show(hourHi, hourLo, minuteHi, minuteLo, secondLedsOn, clever);
}

void displaySetup() { 
  pinMode(SR_OE, OUTPUT);
  digitalWrite(SR_OE, HIGH);   // inverted

  pinMode(SR_RCLK, OUTPUT);
  digitalWrite(SR_RCLK, LOW);

  pinMode(SR_SRCLK, OUTPUT);
  digitalWrite(SR_SRCLK, LOW);

  pinMode(SR_SER, OUTPUT);
  digitalWrite(SR_SER, LOW);

  show(2,0,2,0,true, false); // init screen to 2020 and second leds also on
}