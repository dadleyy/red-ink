#include <SPI.h>
#include <WiFi.h>

#include "board-layout.h"
#include "env.h"
#include "lighting.h"
#include "screen.h"
#include "redis.h"

const unsigned int BOOTING_PHASE_DELAY = 100;

const unsigned int FRAME_BUFFER_SIZE = 1028;
const unsigned int MIN_RECONNECT_WAIT = 10000;

const unsigned int MIN_QUERY_DELAY_TIME = 2000;
const unsigned int MAX_QUERY_RESPONSE_LENGTH = 1048;

const char PROGMEM LPOP_CMD [] = "*2\r\n$4\r\nLPOP\r\n$15\r\nredink:messages";
const char PROGMEM LED_OFF [] = "+led-off";
const char PROGMEM LED_ON [] = "+led-on";

const char PROGMEM MESSAGE_PREPARING_WIFI [] = "preparing wifi...";
const char PROGMEM MESSAGE_WIFI_NOT_FOUND [] = "wifi module not found";
const char PROGMEM MESSAGE_SCANNING [] = "scanning...";
const char PROGMEM MESSAGE_FINISHED_SCAN [] = "scan complete";
const char PROGMEM MESSAGE_DISCONNECTED [] = "disconnected";
const char PROGMEM MESSAGE_FAILED_CONNECTION [] = "failed connection";
const char PROGMEM MESSAGE_CONNECTED [] = "connected";
const char PROGMEM MESSAGE_NO_CONNECTION [] = "no connection";
const char PROGMEM MESSAGE_SSID_NOT_FOUND [] = "ssid not found";

redink::Lighting mc(LED_BUSY_PIN);
redink::Screen screen(
  DISPLAY_DC_PIN,
  DISPLAY_RESET_PIN,
  DISPLAY_CS_PIN,
  DISPLAY_SRAM_CS_PIN,
  DISPLAY_BUSY_PIN
);

enum ELastDisplayReason {
  None = 0,
  ClientMessage = 1,
  Idle = 2,
  Disconnected = 3,
};

enum EConnectionChange {
  UnchangedDisconnect = 0,
  UnchangedConnected = 1,
  EstablishedConnection = 2,
  LostConnection = 3,
};

struct TFrameInfo {
  unsigned long last_reconnect;
  int last_connection_status;

  ELastDisplayReason display_reason;
  char display_buffer [FRAME_BUFFER_SIZE];
  unsigned long display_time;
  unsigned long display_ready;

  unsigned long query_time;
} frame = {
  last_reconnect: 0,
  last_connection_status: WL_IDLE_STATUS,

  display_reason: ELastDisplayReason::None,
  display_buffer: {'\0'},
  display_time: 0,
  display_ready: false,

  query_time: 0,
};

void setup(void) {
  Serial.begin(9600);

  for (unsigned int i = 0; i < 10; i++) {
    mc.booting(i);
    delay(500);
  }

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();

  screen.view(MESSAGE_PREPARING_WIFI);

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();
  WiFi.mode(WIFI_STA);

  mc.ok();
  delay(BOOTING_PHASE_DELAY);
}

void loop(void) {
  unsigned long now = millis();

  // Connection check - if we haven't reconnected, or it has been a while and we are disconnected,
  // we should attempt to reconnect now.
  bool try_reconnect =
    (frame.last_reconnect == 0 || now - frame.last_reconnect > MIN_RECONNECT_WAIT)
    && frame.last_connection_status != WL_CONNECTED;

  if (try_reconnect == true) {
    EConnectionChange result = attemptConnect();
    frame.last_reconnect = now;
  }

  // Display check. See if our display buffer has a message, and if it has been long enough since
  // the last time we updated the display.
  bool has_render = now - frame.display_time > 1000 && frame.display_ready;
  if (has_render == true) {
    screen.view(frame.display_buffer);

    // Clean up our display information.
    memset(frame.display_buffer, '\0', FRAME_BUFFER_SIZE);
    frame.display_ready = false;
    frame.display_time = millis();
  }

  // Update our connection status
  frame.last_connection_status =  WiFi.status();

  if (frame.last_connection_status != WL_CONNECTED) {
    // Only update the display if we didnt just render this message.
    if (frame.display_reason != ELastDisplayReason::Disconnected) {
      frame.display_ready = true;
      frame.display_reason = ELastDisplayReason::Disconnected;
      memset(frame.display_buffer, '\0', FRAME_BUFFER_SIZE);
      memcpy(frame.display_buffer, "nc", 2);
    }

    return;
  }

  if (millis() - frame.query_time < MIN_QUERY_DELAY_TIME) {
    return;
  }

  frame.query_time = millis();
  WiFiClient client;

  if (!client.connect(REDINK_REDIS_HOST, REDINK_REDIS_PORT)) {
    mc.failed();
    frame.display_ready = true;
    frame.display_reason = ELastDisplayReason::Idle;

    memset(frame.display_buffer, '\0', FRAME_BUFFER_SIZE);
    memcpy(frame.display_buffer, "fc", 2);
    client.stop();
    return;
  }

  mc.working();

  client.println(LPOP_CMD);
  delay(100);

  unsigned int len = client.available();

  if (len == 0) {
    client.stop();
    return;
  }

  redink::RedisResponse res;
  unsigned int idx = 0;

  while (idx < len && idx < MAX_QUERY_RESPONSE_LENGTH) {
    char next = client.read();
    res.tick(next);
    idx += 1;
  }

  mc.ok();

  if (res.size() > 0) {
    frame.display_ready = true;
    frame.display_reason = ELastDisplayReason::ClientMessage;
    memset(frame.display_buffer, '\0', FRAME_BUFFER_SIZE);
    res.consume(frame.display_buffer, FRAME_BUFFER_SIZE - 1);
  }

  if (strcmp(frame.display_buffer, LED_ON) == 0) {
    mc.toggle(true);
  } else if (strcmp(frame.display_buffer, LED_OFF) == 0) {
    mc.toggle(false);
  }

  client.stop();
}

EConnectionChange attemptConnect(void) {
  //  Start by scanning for networks that match the SSID configured from our 'env.h' file.
  screen.view(MESSAGE_SCANNING);

  mc.working();
  unsigned long before = millis();
  int count = WiFi.scanNetworks();
  unsigned long after = millis();

  screen.view(MESSAGE_FINISHED_SCAN);

  // Start by scanning for networks that match the SSID configured from our 'env.h' file.
  if (count < 1) {
    mc.failed();
    screen.view(MESSAGE_DISCONNECTED);
    return EConnectionChange::UnchangedDisconnect;
  }

  mc.ok();

  // Attempt to iterate over the number of wifi networks found, maximizing at a reasonable amount.
  for (unsigned char i = 0; i < 255 && i < count; i++) {
    auto ssid = WiFi.SSID(i);

    // If this network does not match the one from our 'env.h', do nothing.
    if (strcmp(ssid.c_str(), REDINK_WIFI_SSID) != 0) {
      continue;
    }

    // At this point, we have a matching ssid, attempt to connect.
    screen.view(ssid.c_str());

    unsigned char attempt = 0;
    int current_status = WL_IDLE_STATUS;

    // Matching SSID connection loop.
    do {
      mc.working();
      current_status = WiFi.begin(REDINK_WIFI_SSID, REDINK_WIFI_PASSWORD);

      if (current_status != WL_CONNECTED) {
        screen.view(MESSAGE_FAILED_CONNECTION);
      } else {
        mc.ok();
        // Here, we have successfully connected, drop out of the connection loop.
        break;
      }

      delay(100);
    } while (current_status != WL_CONNECTED && attempt++ < 10);

    // At this point, we found the matching connection loop and have attempted to connect.
    if (current_status == WL_CONNECTED) {
      screen.view(MESSAGE_CONNECTED);
    } else {
      screen.view(MESSAGE_NO_CONNECTION);
    }

    return current_status == WL_CONNECTED
      ? EConnectionChange::EstablishedConnection
      : EConnectionChange::LostConnection;
  }

  // If no connection attempts were made (we've exhausted our ssid count w/o finding a match),
  // update the display and bow out.

  mc.failed();
  screen.view(MESSAGE_SSID_NOT_FOUND);
  return EConnectionChange::UnchangedDisconnect;
}
