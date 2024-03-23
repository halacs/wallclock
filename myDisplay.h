#ifndef MY_DISPLAY_H
#define MY_DISPLAY_H
#include <Arduino.h>
#include "myLogging.h"

#define DIGIT_OFF 18
#define DIGIT_DASH 19
byte const mapping[] = {
                  0b10000001, // 0
                  0b11001111, // 1
                  0b10010010, // 2
                  0b10000110, // 3
                  0b11001100, // 4
                  0b10100100, // 5
                  0b10100000, // 6
                  0b10001111, // 7
                  0b10000000, // 8
                  0b10000100, // 9
                  0b11111110, // segment #1 only - #10
                  0b11111101, // segment #2 only - #11
                  0b11111011, // segment #3 only - #12
                  0b11110111, // segment #4 only - #13
                  0b11101111, // segment #5 only - #14
                  0b11011111, // segment #6 only - #15
                  0b10111111, // segment #7 only - #16
                  0b10000000, // full digit ON   - #17
                  0b11111111, // full digit OFF  - #18
                  0b11111110, // dash            - #19
                };

byte const mappingClever[] = {
                  0b10000001, // 0
                  0b11111001, // 1
                  0b10010010, // 2
                  0b10000110, // 3
                  0b11001100, // 4
                  0b10100100, // 5
                  0b10100000, // 6
                  0b10001111, // 7
                  0b10000000, // 8
                  0b10000100, // 9
                  0b11111110, // segment #1 only - #10
                  0b11111101, // segment #2 only - #11
                  0b11111011, // segment #3 only - #12
                  0b11110111, // segment #4 only - #13
                  0b11101111, // segment #5 only - #14
                  0b11011111, // segment #6 only - #15
                  0b10111111, // segment #7 only - #16
                  0b10000000, // full digit ON   - #17
                  0b11111111, // full digit OFF  - #18
                  0b11111110, // dash            - #19
                };

void syncDisplayScheduler();
void updateDisplay();
void displaySetup();

#endif