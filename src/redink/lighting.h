#ifndef _REDINK_LIGHTING_H
#define _REDINK_LIGHTING_H

#include <Arduino.h>

namespace redink {
  class Lighting {
    public:
      Lighting(unsigned int);

      void ok(void);
      void working(void);
      void failed(void);
      void booting(unsigned int);

      void toggle(bool);

    private:
      bool _toggled;
      unsigned int _busy;
  };

}

#endif
