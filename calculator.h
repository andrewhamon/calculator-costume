typedef struct {
  char   display[CALCULATOR_BUFFER_SIZE];
  char   input[CALCULATOR_BUFFER_SIZE];
  double value;
  char   current_op;
  size_t display_width;
} calculator_t;

