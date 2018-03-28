#define CALCULATOR_BUFFER_SIZE 128


class Calculator
{
  public:
  Calculator(int display_width);
  void reset();
  void input(char c);
  const char * display();
  double value;
  char   input_buffer[CALCULATOR_BUFFER_SIZE];
  void (*calculator_state_callback)(char input);


  private:
  char   display_buffer[CALCULATOR_BUFFER_SIZE];
  char   current_op;
  int display_width;
  bool input_buffer_full();
  void handle_op();
  void show_value();
  void show_input_buffer();
  void clear_input_buffer();
  void append_to_input_buffer(char c);
};