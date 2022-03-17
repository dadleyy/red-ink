#ifndef _REDINK_SCREEN_H
#define _REDINK_SCREEN_H 1

#include <GxEPD2_BW.h>

namespace redink {

  class Screen {
    public:
      Screen(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
      void view(const char *);

    private:
      GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> _display;
      bool _booted;
  };

}

#endif
