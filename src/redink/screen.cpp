#include "screen.h"

redink::Screen::Screen(unsigned int dc, unsigned int reset, unsigned int cs, unsigned int sram, unsigned int busy):
  _display(dc, reset, cs, sram, busy),
  _booted(false) {
}

void redink::Screen::view(const char * message) {
  if (_booted == false) {
    _booted = true;
    _display.begin(THINKINK_MONO);
    _display.setTextColor(EPD_BLACK);
    _display.setTextSize(2);
  }

  _display.clearBuffer();
  _display.setCursor(0, 0);
  _display.println(message);
  _display.display();
}
