#ifndef _JOY_REDIS_H
#define _JOY_REDIS_H 1

namespace netdisplay {

  enum ResponsePhase {
    Empty,
    ArrayLength,
    ArrayItemPrebuffer,
    ArrayItemBuffer,
    Complete,
    Failed,
  };

  class RedisResponse {
    public:
      RedisResponse();
      void tick(char);
      unsigned int size(void);
      bool consume(char *, unsigned int);

    private:
      static const unsigned int REDIS_FRAME_BUFFER_SIZE = 1024;

      char _buffer [REDIS_FRAME_BUFFER_SIZE];
      unsigned int _position;
      unsigned int _content_len;
      ResponsePhase _phase;
  };

}

#endif
