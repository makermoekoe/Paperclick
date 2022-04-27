# Paperclick

Paperclick is a simple device which combines a [Picoclick-C3T](https://github.com/makermoekoe/Picoclick-C3) with the smallest EPaper display I found in the internet: [a 1.02" 128x80 pixels EPD](https://www.waveshare.com/wiki/1.02inch_e-paper_Module). The initial task was to create a simple activity tracker which updates the EPaper display with the current date and time once the display is pressed (actually it is not the display sensing the press, it's a button behind the display). The device gets the time from an NTP server over WiFi.

Paperclick is based on the single core ESP32-C3 processor which is used in the Picoclick C3T as well. Furthermore it comes with an optimized battery management which brings the complete device down to around 200nA current consumption in idle state.

# Features

- ESP32-C3 MCU with onboard antenna
- 1.02" 128x80 pixel Epaper display
- Onboard battery charger
- Optimized battery capacity estimation
- Simple One-Button user interface (Two optional buttons available)

# Battery management

# Assembled devive
