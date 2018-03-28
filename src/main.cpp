#include <Adafruit_MPR121.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "bluefruit_config.h"

#include <Arduino.h>

#include <calculator.h>
#include <display.h>
#include <main.h>


#define DISPLAY_DATA_PIN  4
#define DISPLAY_CLOCK_PIN 5
#define DISPLAY_LATCH_PIN 6

#define DISPLAY_WIDTH     6

#define TOUCH_SENSOR_1_ADDR 0x5A
#define TOUCH_SENSOR_1_IRQ_PIN 3
uint16_t sensor_1_last_touched    = 0;
uint16_t sensor_1_current_touched = 0;
bool sensor_1_irq = 0;
Adafruit_MPR121 touch_sensor_1 = Adafruit_MPR121();

#define TOUCH_SENSOR_2_ADDR 0x5B
#define TOUCH_SENSOR_2_IRQ_PIN 2
uint16_t sensor_2_last_touched = 0;
uint16_t sensor_2_current_touched = 0;
bool sensor_2_irq = 0;
Adafruit_MPR121 touch_sensor_2 = Adafruit_MPR121();

char incomingByte;

Display display = Display(DISPLAY_DATA_PIN, DISPLAY_CLOCK_PIN, DISPLAY_LATCH_PIN, DISPLAY_WIDTH);
Calculator calculator = Calculator(DISPLAY_WIDTH);

SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);

Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);


void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}



void setup(){
  calculator.calculator_state_callback = on_calculator_state_change;
  display.begin();

  Serial.begin(115200);

  if (!touch_sensor_1.begin(TOUCH_SENSOR_1_ADDR)) {
    Serial.println("Touch sensor 1 not found");
    while(1);
  }
  Serial.println("Touch sensor 1 found!");

  if (!touch_sensor_2.begin(TOUCH_SENSOR_2_ADDR)) {
    Serial.println("Touch sensor 2 not found");
    while(1);
  }
  Serial.println("Touch sensor 2 found!");


  display.push_string(calculator.display());

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  // /* Wait for connection */
  // while (! ble.isConnected()) {
  //     delay(500);
  // }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));
}

void on_touch_press(char digit){
  calculator.input(digit);
  display.push_string(calculator.display());
  Serial.print(digit); Serial.println(" touched");
}

void process_touch_sensor_1(){
  sensor_1_current_touched = touch_sensor_1.touched();

  for (char i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((sensor_1_current_touched & _BV(i)) && !(sensor_1_last_touched & _BV(i)) ) {
      on_touch_press(touch_to_char(TOUCH_SENSOR_1_ADDR, i));
    }
  }

  sensor_1_last_touched = sensor_1_current_touched;
}

void process_touch_sensor_2(){
  sensor_2_current_touched = touch_sensor_2.touched();

  for (char i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((sensor_2_current_touched & _BV(i)) && !(sensor_2_last_touched & _BV(i)) ) {
      on_touch_press(touch_to_char(TOUCH_SENSOR_2_ADDR, i));
    }
  }

  sensor_2_last_touched = sensor_2_current_touched;
}

char touch_to_char(uint8_t addr, uint8_t pin){
  if (addr == TOUCH_SENSOR_1_ADDR){
    switch(pin){
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        return pin + '0';
      case 10:
      case 11:
      default:
        return ' ';
    }
  }

  if (addr == TOUCH_SENSOR_2_ADDR){
    switch(pin){
      case 0:
        return 'c';
      case 1:
        return '.';
      case 2:
        return '=';
      case 3:
        return '+';
      case 4:
        return '-';
      case 5:
        return '*';
      case 6:
        return '/';
      case 7:
        return 's';
      case 8:
        return '^';
      case 9:
      case 10:
      case 11:
      default:
        return ' ';
    }
  }

  return ' ';
}

void on_calculator_state_change(char input){
  if(input != '='){
    return;
  }



  int digits = 0;
  for(int i = 0; i < strlen(calculator.input_buffer); i++){
    if(calculator.input_buffer[i] < '0' || calculator.input_buffer[i] > '9'){
      return;
    }

    digits++;
  }


  if(digits == 10){
    Serial.print("Possible phone number encountered: "); Serial.println(calculator.input_buffer);
    ble.println(calculator.input_buffer);
  }
}

void loop()
{
  process_touch_sensor_1();
  process_touch_sensor_2();

  while(Serial.available()) {
    incomingByte = Serial.read();
    if (isWhitespace(incomingByte)) {
      return;
    }
    calculator.input(incomingByte);
    display.push_string(calculator.display());
    Serial.println(calculator.display());
  }
}

