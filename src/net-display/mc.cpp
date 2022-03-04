#include "mc.h"

netdisplay::Mc::Mc(int data, int clock, int mode): dot(1, data, clock, mode) {
}

void netdisplay::Mc::ok(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(0, 255, 0));
  dot.show();
}

void netdisplay::Mc::working(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(0, 0, 255));
  dot.show();
}

void netdisplay::Mc::booting(unsigned int i) {
  if (i == 0) {
    dot.begin();
  }

  unsigned int based = i % 100;
  unsigned int brightness = based > 50 ? 50 - (based - 50) : based;
  dot.setBrightness(brightness);
  dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  dot.show();
}

void netdisplay::Mc::failed(void) {
  dot.clear();
  dot.setBrightness(60);
  dot.setPixelColor(0, Adafruit_DotStar::Color(255, 0, 0));
  dot.show();
}
