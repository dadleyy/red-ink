#include "lighting.h"

redink::Lighting::Lighting(unsigned int data, unsigned int clock, unsigned int mode):
  _dot(1, data, clock, mode),
  _toggled(true) {
}

void redink::Lighting::ok(void) {
  if (_toggled == false) {
    return;
  }

  _dot.clear();
  _dot.setBrightness(60);
  _dot.setPixelColor(0, Adafruit_DotStar::Color(0, 255, 0));
  _dot.show();
}

void redink::Lighting::toggle(bool on) {
  _toggled = on;

  if (on == false) {
    _dot.clear();
    _dot.setBrightness(0);
    _dot.show();
  }
}

void redink::Lighting::working(void) {
  if (_toggled == false) {
    return;
  }

  _dot.clear();
  _dot.setBrightness(60);
  _dot.setPixelColor(0, Adafruit_DotStar::Color(0, 0, 255));
  _dot.show();
}

void redink::Lighting::booting(unsigned int i) {
  if (i == 0) {
    _dot.begin();
  }

  unsigned int based = i % 100;
  unsigned int brightness = based > 50 ? 50 - (based - 50) : based;
  _dot.setBrightness(brightness);
  _dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  _dot.show();
}

void redink::Lighting::failed(void) {
  if (_toggled == false) {
    return;
  }

  _dot.clear();
  _dot.setBrightness(60);
  _dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  _dot.show();
}
