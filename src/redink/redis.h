#ifndef _REDINK_REDIS_H
#define _REDINK_REDIS_H 1

namespace redink {

  enum ERedisResponsePhase {
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
      ERedisResponsePhase _phase;
  };

}

#endif
