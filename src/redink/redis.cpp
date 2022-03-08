#include "redis.h"
#include <cstring>

redink::RedisResponse::RedisResponse():
  _position(0),
  _content_len(0),
  _phase(ERedisResponsePhase::Empty) {
  memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
}

void redink::RedisResponse::tick(char token) {
  // If we're currently in a terminal state, do nothing.
  if (_phase == ERedisResponsePhase::Failed || _phase == ERedisResponsePhase::Complete) {
    return;
  }

  // Start of message; length or failed.
  if (_phase == ERedisResponsePhase::Empty) {
    _phase = token == '$' ? ERedisResponsePhase::ArrayLength : ERedisResponsePhase::Failed;
    return;
  }

  // Once we're getting array length tokens, push them into our buffer.
  if (_phase == ERedisResponsePhase::ArrayLength && token != '\r') {
    _buffer[_position] = token;
    _position += 1;

    if (_position >= REDIS_FRAME_BUFFER_SIZE) {
      _phase = ERedisResponsePhase::Failed;
    }
    return;
  }

  // If we're dealing with the array length and we're on a carriage return,
  // we have a buffer that contains the string version of our response length.
  if (_phase == ERedisResponsePhase::ArrayLength && token == '\r') {

    // Handle -1 (empty) responses explicitly.
    if (strcmp(_buffer, "-1") == 0) {
      memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
      _position = 0;
      _phase = ERedisResponsePhase::Complete;
      return;
    }

    for (unsigned int i = 0; i < _position; i++) {
      _content_len = (_content_len * 10) + (_buffer[i] - '0');
    }

    // Now, repurpose our buffer so we can be ready to cursor tokens.
    memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
    _position = 0;
    _phase = ERedisResponsePhase::ArrayItemPrebuffer;
    return;
  }

  if (_phase == ERedisResponsePhase::ArrayItemPrebuffer) {
    memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
    _phase = token == '\n' ? ERedisResponsePhase::ArrayItemBuffer : ERedisResponsePhase::Failed;
    _position = 0;
    return;
  }

  if (_phase == ERedisResponsePhase::ArrayItemBuffer) {
    _buffer[_position] = token;
    _position += 1;

    // If we're not at the position of our content length, we are complete.
    if (_position == _content_len) {
      _phase = ERedisResponsePhase::Complete;
    }
  }
}

bool redink::RedisResponse::consume(char * destination, unsigned int len) {
  if (_phase != ERedisResponsePhase::Complete || _content_len == 0) {
    return false;
  }

  for (unsigned int i = 0; i < _content_len && i < len; i++) {
    destination[i] = _buffer[i];
  }

  return true;
}

unsigned int redink::RedisResponse::size(void) {
  return _content_len;
}
