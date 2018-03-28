#include <Arduino.h>
#include <display.h>

Display::Display(int dp, int cp, int lp, int w){
  data_pin = dp; clock_pin = cp; latch_pin = lp; width = w;
}

void Display::begin(){
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  pinMode(latch_pin, OUTPUT);

  push_string("");
}

void Display::push_raw(uint8_t data){
  shiftOut(data_pin, clock_pin, MSBFIRST, data);
}

void Display::latch(){
  digitalWrite(latch_pin, LOW);
  digitalWrite(latch_pin, HIGH); //Register moves storage register on the rising edge of RCK
}

void Display::push_char(char num, boolean decimalPoint){
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

  uint8_t segments = 0;

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
    case '-': segments = SEG_G; break;
    case ' ': segments = 0; break;
  }

  if (decimalPoint) segments |= SEG_DP;

  push_raw(segments);
}

void Display::push_string(const char * str){
  int len = strlen(str);

  int will_display = 0;
  int visible_chars_encountered = 0;
  char current_char;
  while(visible_chars_encountered < width && will_display < len){
    current_char = str[will_display];

    if(current_char != '.'){
      visible_chars_encountered++;
    }
    will_display++;
  }

  bool char_has_dp = false;
  while(will_display > 0){
    will_display--;
    current_char = str[will_display];

    if(current_char == '.'){
      char_has_dp = true;
    } else {
      push_char(current_char, char_has_dp);
      char_has_dp = false;
    }
  }

  if(visible_chars_encountered < width){
    for (int i = 0; i < (width - visible_chars_encountered); i++) {
      push_char(' ', false);
    }
  }

  latch();
}
