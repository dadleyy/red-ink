#ifndef _REDINK_LIGHTING_H
#define _REDINK_LIGHTING_H

#include "Adafruit_DotStar.h"

namespace redink {
  class Lighting {
    public:
      Lighting(unsigned int, unsigned int, unsigned int);

      void ok(void);
      void working(void);
      void failed(void);
      void booting(unsigned int);

      void toggle(bool);

    private:
      Adafruit_DotStar _dot;
      bool _toggled;
  };

}

#endif
