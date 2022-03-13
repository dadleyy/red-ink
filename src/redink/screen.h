#ifndef _REDINK_SCREEN_H
#define _REDINK_SCREEN_H 1

#include "Adafruit_ThinkInk.h"

namespace redink {

  class Screen {
    public:
      Screen(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
      void view(const char *);

    private:
      ThinkInk_154_Tricolor_Z90 _display;
      bool _booted;
  };

}

#endif
