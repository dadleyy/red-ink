#include "lighting.h"

redink::Lighting::Lighting(unsigned int busy):
  _toggled(true),
  _busy(busy) {
}

void redink::Lighting::ok(void) {
  if (_toggled == false) {
    return;
  }

  digitalWrite(_busy, LOW);
}

void redink::Lighting::toggle(bool on) {
  _toggled = on;

  if (on == false) {
    digitalWrite(_busy, LOW);
  }
}

void redink::Lighting::working(void) {
  if (_toggled == false) {
    return;
  }

  digitalWrite(_busy, HIGH);
}

void redink::Lighting::booting(unsigned int i) {
  if (i == 0) {
    pinMode(_busy, OUTPUT);
  }

  digitalWrite(_busy, i % 2 == 0 ? HIGH : LOW);
}

void redink::Lighting::failed(void) {
  if (_toggled == false) {
    return;
  }

  digitalWrite(_busy, HIGH);
}
