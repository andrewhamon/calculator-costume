#include <Adafruit_MPR121.h>
#include <Wire.h>
#include <calculator.cpp>

/*
 Controlling large 7-segment displays
 By: Nathan Seidle
 SparkFun Electronics
 Date: February 25th, 2015
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 The large 7 segment displays can be controlled easily with a TPIC6C594 IC. This code demonstrates how to control
 one display.
 Here's how to hook up the Arduino pins to the Large Digit Driver
 
 Arduino pin 6 -> CLK (Green on the 6-pin cable)
 5 -> LAT (Blue)
 7 -> SER on the IN side (Yellow)
 5V -> 5V (Orange)
 Power Arduino with 12V and connect to Vin -> 12V (Red)
 GND -> GND (Black)
 There are two connectors on the Large Digit Driver. 'IN' is the input side that should be connected to
 your microcontroller (the Arduino). 'OUT' is the output side that should be connected to the 'IN' of addtional
 digits.
 
 Each display will use about 150mA with all segments and decimal point on.
*/

#define DISPLAY_DATA_PIN  7
#define DISPLAY_CLOCK_PIN 6
#define DISPLAY_LATCH_PIN 5
#define DISPLAY_WIDTH     6

#define DIGITS_IRQ_PIN 2
#define DIGITS_ADDR 0x5A

#define OPS_IRQ_PIN 3
#define OPS_ADDR 0x5B


uint16_t digitsLastTouched = 0;
uint16_t digitsCurrTouched = 0;
bool     digitsInterrupted = 0;
Adafruit_MPR121 digitsTouchPad = Adafruit_MPR121();

// uint16_t opsLastTouched = 0;
// uint16_t opsCurrTouched = 0;
// bool     opsInterrupted = 0;
// Adafruit_MPR121 opsTouchPad = Adafruit_MPR121();


calculator_t calculator;

uint8_t incomingByte;




//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup(){
  Serial.begin(9600);
  pinMode(DISPLAY_CLOCK_PIN, OUTPUT);
  pinMode(DISPLAY_DATA_PIN, OUTPUT);
  pinMode(DISPLAY_LATCH_PIN, OUTPUT);

  pushString("");

  if (!digitsTouchPad.begin(DIGITS_ADDR)) {
    Serial.println("Digits touchpad not found");
    while(1);
  }
  Serial.println("Digits touchpad found!");

  pinMode(DIGITS_IRQ_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DIGITS_IRQ_PIN), on_digit_irq, FALLING);

  // if (!opsTouchPad.begin(OPS_ADDR)) {
  //   Serial.println("Operations touchpad not found");
  //   while(1);
  // }
  // Serial.println("Operations touchpad found!");

  // pinMode(OPS_IRQ_PIN, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(OPS_IRQ_PIN), onOpIRQ, FALLING);

  calculator_init(&calculator);
  pushString(calculator.display);
}

void on_digit_irq(){
  digitsInterrupted = true;
}

// void onOpIRQ(){
//   digitsInterrupted = true;
// }

void processDigits(){
  // Get the currently touched pads
  digitsCurrTouched = digitsTouchPad.touched();
  
  for (uint8_t i=0; i<10; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((digitsCurrTouched & _BV(i)) && !(digitsLastTouched & _BV(i)) ) {
      onDigitDown(i);
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(digitsCurrTouched & _BV(i)) && (digitsLastTouched & _BV(i)) ) {
      onDigitUp(i);
    }
  }

  // reset our state
  digitsLastTouched = digitsCurrTouched;
}

void onDigitDown(uint8_t digit){
  Serial.print(digit); Serial.println(" touched");
}

void onDigitUp(uint8_t digit){
  Serial.print(digit); Serial.println(" released");
  calculator_input(&calculator, digit + '0');
  pushString(calculator.display);
}

// void processOps(){

// }

void loop()
{
  if (digitsInterrupted) {
    processDigits();
    digitsInterrupted = false;
  }

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (isWhitespace(incomingByte)) {
      return;
    }
    calculator_input(&calculator, incomingByte);
    pushString(calculator.display);
    Serial.println(calculator.display);
    Serial.println(calculator.value);
  }

  // if (opsInterrupted) {
  //   processOps()
  //   opsInterrupted = false
  // }
}

//Given a number, or '-', shifts it out to the display
void pushChar(char num, boolean decimalPoint)
{
  //    -  A
  //   / / F/B
  //    -  G
  //   / / E/C
  //    -. D/DP

  #define SEG_A  1<<0
  #define SEG_B  1<<6
  #define SEG_C  1<<5
  #define SEG_D  1<<4
  #define SEG_E  1<<3
  #define SEG_F  1<<1
  #define SEG_G  1<<2
  #define SEG_DP 1<<7

  byte segments;

  switch (num)
  {
    case '1': segments = SEG_B | SEG_C; break;
    case '2': segments = SEG_A | SEG_B | SEG_D | SEG_E | SEG_G; break;
    case '3': segments = SEG_A | SEG_B | SEG_C | SEG_D | SEG_G; break;
    case '4': segments = SEG_F | SEG_G | SEG_B | SEG_C; break;
    case '5': segments = SEG_A | SEG_F | SEG_G | SEG_C | SEG_D; break;
    case '6': segments = SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D; break;
    case '7': segments = SEG_A | SEG_B | SEG_C; break;
    case '8': segments = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G; break;
    case '9': segments = SEG_A | SEG_B | SEG_C | SEG_F | SEG_G; break;
    case '0': segments = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F; break;
    case ' ': segments = 0; break;
    case 'c': segments = SEG_G | SEG_E | SEG_D; break;
    case '-': segments = SEG_G; break;
  }

  if (decimalPoint) segments |= SEG_DP;

  digitalWrite(DISPLAY_LATCH_PIN, HIGH);
  shiftOut(DISPLAY_DATA_PIN, DISPLAY_CLOCK_PIN, MSBFIRST, segments);

}

void pushString(uint8_t * str){
  uint8_t digit;
  uint8_t isDp;

  size_t len = strlen(str);
  size_t numWritten = 0;

  for (size_t i = len; i > 0; i--){
    digit = str[i-1];

    if (digit == '.' && i == 1) {
      pushChar('0', true);
      numWritten++;
      continue;
    }

    if (digit == '.') {
      continue;
    }

    isDp = (str[i] == '.');

    pushChar(digit, isDp);
    numWritten++;
  }

  if (numWritten < DISPLAY_WIDTH) {
    for (size_t i = 0; i < (DISPLAY_WIDTH - numWritten); i++) {
      pushChar(' ', false);
    }
  }

  digitalWrite(DISPLAY_LATCH_PIN, LOW);
  digitalWrite(DISPLAY_LATCH_PIN, HIGH); //Register moves storage register on the rising edge of RCK
}

size_t maxDisplayFit(uint8_t * str) {
  size_t found = 0;
  uint8_t curr = 0;

  while(1){
    curr = &str;
    if (curr == NULL)
      return found;

    if (isDisplayable(curr))
      found++;

    if(found >= DISPLAY_WIDTH)
      return found;
  }
}

boolean isDisplayable(uint8_t c){
  switch(c){
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
    case ' ':
    case 'c':
    case '-':
      return true;
    default:
      return false;
  }
}