class Display
{
  public:
  Display(int data_pin, int clock_pin, int latch_pin, int display_width);
  void latch();
  void push_raw(uint8_t);
  void push_char(char, bool);
  void push_string(const char *);
  void begin();

  private:
  int data_pin, clock_pin, latch_pin, width;
};
