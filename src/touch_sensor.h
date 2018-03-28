class TouchSensor
{
  public:
  TouchSensor(uint8_t, uint8_t, void (*on_touch_press)(uint8_t, uint8_t));
  bool begin();
  void process();


  private:
  uint16_t current_touched, previous_touched, address, irq_pin;

  void (*on_touch_press)(uint8_t, uint8_t);
};
