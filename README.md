# Calculator Costume
My over-the-top Halloween costume.

<img src="https://user-images.githubusercontent.com/5134584/38205192-ad210462-3673-11e8-89c2-e6e046ff9990.jpeg" alt="img_4266" width="400">

# Features
- A real, working basic calculator.
- Tries hard to mimic all calculator behaviors (i.e. you can type 5 + 5 + 5 + 5 and the total keeps updating, just like a basic calculator).
- Display will scroll with large inputs.

# Bugs
- Can not be self-operated while wearing -- requires a second person.
- Limited precision.
- Only 6 digit display.
- Stopped working after a few hours downtown - most likely had a few too many drinks spilled on it (its quite large, lots of bumping in to people), or a short from my shoddy soldering skills. It was basically a completely exposed proto-board.
- Limited mobility while wearing.

# Easter Eggs
- BLE module that communicated with an iOS app I whipped up (code not up yet, I am _not_ an iOS developer by any means). If a sequence of numbers was entered that looked like a phone number, my phone would alert me and give me the opportunity to send a text.
  - Successfully acquired exactly one phone number this way before the costume died.
    - I regret nothing.

# Implementation details
- Micro-controller was an [Adafruit Metro Mini](https://www.adafruit.com/product/2590)
- Giant 7-segment LEDs were [from Sparkfun](https://www.sparkfun.com/products/8530) and were driven by [a handy backpack board](https://www.sparkfun.com/products/13279)
  - Controlling the display is essentially controlling a shift register
- Capacitive buttons were implemented with copper tape, wire, and a couple of these [Adafruit capacitive sensing breakouts](https://www.adafruit.com/product/1982)
- Bluetooth was the [Adafruit BLE UART friend](https://www.adafruit.com/product/2479) + software serial
