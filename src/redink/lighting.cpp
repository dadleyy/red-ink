#include "lighting.h"

redink::Lighting::Lighting(unsigned int data, unsigned int clock, unsigned int mode):
  dot(1, data, clock, mode) {
}

void redink::Lighting::ok(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(0, 255, 0));
  dot.show();
}

void redink::Lighting::working(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(0, 0, 255));
  dot.show();
}

void redink::Lighting::booting(unsigned int i) {
  if (i == 0) {
    dot.begin();
  }

  unsigned int based = i % 100;
  unsigned int brightness = based > 50 ? 50 - (based - 50) : based;
  dot.setBrightness(brightness);
  dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  dot.show();
}

void redink::Lighting::failed(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  dot.show();
}
