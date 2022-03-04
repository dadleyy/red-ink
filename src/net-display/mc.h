#ifndef _NET_DISPLAY_MC_H
#define _NET_DISPLAY_MC_H

#include "Adafruit_DotStar.h"

namespace netdisplay {

  class Mc {
    public:
      Mc(int data, int clock, int mode);

      void ok(void);
      void working(void);
      void failed(void);
      void booting(unsigned int);

    private:
      Adafruit_DotStar dot;
  };

}

#endif
