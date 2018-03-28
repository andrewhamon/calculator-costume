#include <Arduino.h>
#include <calculator.h>

void claculatorCallbackPlaceholderDoNothing(char c){
  return;
}

Calculator::Calculator(int width) {
  calculator_state_callback = claculatorCallbackPlaceholderDoNothing;
  display_width = width;
  reset();
}

void Calculator::reset(){
  display_buffer[0] = '\0';
  input_buffer[0] = '\0';
  value = 0.0;
  current_op = '\0';
  show_value();
}

bool Calculator::input_buffer_full() {
  return strlen(input_buffer) >= (CALCULATOR_BUFFER_SIZE - 1);
}

void Calculator::handle_op() {
  double parsed_value = strtod(input_buffer, NULL);

  switch(current_op){
    case '+':
      value += parsed_value;
      break;
    case '-':
      value -= parsed_value;
      break;
    case '*':
      value *= parsed_value;
      break;
    case '/':
      value /= parsed_value;
      break;
    case '^':
      value = pow(value, parsed_value);
      break;
    case 0:
      value = parsed_value;
      break;
  }
}

void Calculator::show_value(){
  // Print current val into display_buffer
  dtostrf(value, 0, display_width, display_buffer);

  char * dec_ptr = strchr(display_buffer, '.');

  // This should never happen, but give up if it does
  if (dec_ptr == NULL) {
    return;
  }

  // Get length of number without decimal part
  int int_part_len   = dec_ptr - display_buffer;
  int decimal_digits = display_width - int_part_len;


  // If we have room for decimal part, round off, otherwise omit decimal
  if (decimal_digits > 0) {
    dtostrf(value, 0, decimal_digits, display_buffer);
  } else {
    dtostrf(value, 0, 0, display_buffer);
  }


  // Once again search for the decimal
  dec_ptr = strchr(display_buffer, '.');

  // Return if we don't find it
  if (dec_ptr == NULL) {
    return;
  }

  // Trim trailing zeroes
  for (int i = decimal_digits; decimal_digits > 0; i--) {
    if (dec_ptr[i] == '0') {
      dec_ptr[i] = '\0';
    } else {
      break;
    }
  }

  // Ensure no trailing decimal
  if (dec_ptr[1] == '\0') {
    dec_ptr[0] = '\0';
  }
}

void Calculator::show_input_buffer(){
  int count = 0;
  int str_len = strlen(input_buffer);

  while(str_len > 0 && count < display_width){
    if(input_buffer[str_len - 1] != '.'){
      count++;
    }
    str_len--;
  }

  strcpy(display_buffer, input_buffer + str_len);
}

void Calculator::clear_input_buffer(){
  input_buffer[0] = '\0';
}

void Calculator::append_to_input_buffer(char c){
  int str_len = strlen(input_buffer);
  if (!input_buffer_full()) {
    input_buffer[str_len] = c;
    input_buffer[str_len + 1] = '\0';
  }
}

void Calculator::input(char c) {
  if (c == '\n' || c == '\r') {
    c = '=';
  }

  switch(c) {
    case '.':
      // Ignore duplicate decimals
      if(strchr(input_buffer, '.'))
        break;
      append_to_input_buffer(c);
      show_input_buffer();
      calculator_state_callback(c);
      break;

    case '0':
      // Prevent leading zeros
      if(strlen(input_buffer) == 1 && input_buffer[0] == '0')
        break;

      // Even if there is a negative
      if(strlen(input_buffer) == 2 && input_buffer[0] == '-' && input_buffer[1] == '0')
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      // Replace "-0" with "-{digit}"
      if(strlen(input_buffer) == 2 && input_buffer[0] == '-' && input_buffer[1] == '0')
        input_buffer[1] = '\0';
      // If a new number follows = rather than another op, discard old data
      if (current_op == '=') {
        reset();
      }

      append_to_input_buffer(c);
      show_input_buffer();
      calculator_state_callback(c);
      break;
    case '-':
      // If input_buffer is empty, assume - is a negative sign rather than an operation
      if(strlen(input_buffer) == 0 && current_op != '=') {
        append_to_input_buffer(c);
        show_input_buffer();
        calculator_state_callback(c);
        break;
      }

      // Undo negative if its a duplicate
      if(strlen(input_buffer) == 1 && input_buffer[0] == '-'){
        input_buffer[0] = '\0';
        show_input_buffer();
        calculator_state_callback(c);
        break;
      }

    case '+':
    case '*':
    case '/':
    case '^':
    case '=':
      handle_op();
      current_op = c;
      show_value();
      calculator_state_callback(c);
      clear_input_buffer();
      break;
    case 'c':
      reset();
      calculator_state_callback(c);
      break;
  }
}

const char * Calculator::display(){
  return display_buffer;
}
