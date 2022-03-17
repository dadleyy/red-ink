#include "screen.h"

redink::Screen::Screen(
    unsigned int dc,
    unsigned int reset,
    unsigned int cs,
    unsigned int sram,
    unsigned int busy):
  _display(GxEPD2_290_T94(cs, dc, reset, busy)),
  _booted(false) {
}

void redink::Screen::view(const char * message) {
  if (_booted == false) {
    _booted = true;
    _display.init();
    _display.setTextColor(GxEPD_BLACK);
    _display.setTextSize(2);
    _display.setRotation(1);
  }

  _display.fillScreen(GxEPD_WHITE);
  _display.setCursor(0, 0);
  _display.println(message);
  _display.display();
}
