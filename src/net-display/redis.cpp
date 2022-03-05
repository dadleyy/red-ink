#include "redis.h"
#include <cstring>

netdisplay::RedisResponse::RedisResponse():
  _position(0),
  _content_len(0),
  _phase(ResponsePhase::Empty) {
  memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
}

void netdisplay::RedisResponse::tick(char token) {
  // If we're currently in a terminal state, do nothing.
  if (_phase == ResponsePhase::Failed || _phase == ResponsePhase::Complete) {
    return;
  }

  // Start of message; length or failed.
  if (_phase == ResponsePhase::Empty) {
    _phase = token == '$' ? ResponsePhase::ArrayLength : ResponsePhase::Failed;
    return;
  }

  // Once we're getting array length tokens, push them into our buffer.
  if (_phase == ResponsePhase::ArrayLength && token != '\r') {
    _buffer[_position] = token;
    _position += 1;

    if (_position >= REDIS_FRAME_BUFFER_SIZE) {
      _phase = ResponsePhase::Failed;
    }
    return;
  }

  // If we're dealing with the array length and we're on a carriage return,
  // we have a buffer that contains the string version of our response length.
  if (_phase == ResponsePhase::ArrayLength && token == '\r') {

    // Handle -1 (empty) responses explicitly.
    if (strcmp(_buffer, "-1") == 0) {
      memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
      _position = 0;
      _phase = ResponsePhase::Complete;
      return;
    }

    for (unsigned int i = 0; i < _position; i++) {
      _content_len = (_content_len * 10) + (_buffer[i] - '0');
    }

    // Now, repurpose our buffer so we can be ready to cursor tokens.
    memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
    _position = 0;
    _phase = ResponsePhase::ArrayItemPrebuffer;
    return;
  }

  if (_phase == ResponsePhase::ArrayItemPrebuffer) {
    memset(_buffer, '\0', REDIS_FRAME_BUFFER_SIZE);
    _phase = token == '\n' ? ResponsePhase::ArrayItemBuffer : ResponsePhase::Failed;
    _position = 0;
    return;
  }

  if (_phase == ResponsePhase::ArrayItemBuffer) {
    _buffer[_position] = token;
    _position += 1;

    // If we're not at the position of our content length, we are complete.
    if (_position == _content_len) {
      _phase = ResponsePhase::Complete;
    }
  }
}

bool netdisplay::RedisResponse::consume(char * destination, unsigned int len) {
  if (_phase != ResponsePhase::Complete || _content_len == 0) {
    return false;
  }

  for (unsigned int i = 0; i < _content_len && i < len; i++) {
    destination[i] = _buffer[i];
  }

  return true;
}

unsigned int netdisplay::RedisResponse::size(void) {
  return _content_len;
}
