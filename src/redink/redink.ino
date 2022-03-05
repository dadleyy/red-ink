#include <SPI.h>
#include <WiFiNINA.h>
#include "Adafruit_ThinkInk.h"

#include "board-layout.h"
#include "env.h"
#include "mc.h"
#include "redis.h"

const unsigned int BOOTING_PHASE_DELAY = 100;

const unsigned int FRAME_BUFFER_SIZE = 1028;
const unsigned int MIN_RECONNECT_WAIT = 10000;


const unsigned int MIN_QUERY_DELAY_TIME = 5000;
const unsigned int MAX_QUERY_RESPONSE_LENGTH = 1048;

redink::Mc mc(DOTSTAR_DATA_PIN, DOTSTAR_CLOCK_PIN, DOTSTAR_BGR);

ThinkInk_290_Mono_M06 display(
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

  for (unsigned int i = 0; i < 500; i++) {
    mc.booting(i);
    delay(10);
  }

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();
  display.begin(THINKINK_MONO);

  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);

  display.clearBuffer();
  display.setCursor(0, 0);
  display.print("Preparing WiFi...");
  display.display();

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();
  WiFi.setPins(WIFI_CS_PIN, WIFI_BUSY_PIN, WIFI_RESET_PIN, WIFI_GPIO_PIN, &SPI);

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();
  while (WiFi.status() == WL_NO_MODULE) {
    mc.failed();
    delay(1000);
  }

  mc.ok();
  delay(BOOTING_PHASE_DELAY);

  mc.working();
  
  byte mac[6];
  String fv = WiFi.firmwareVersion();
  WiFi.macAddress(mac);

  display.clearBuffer();
  display.setCursor(0, 0);
  display.println(fv);
  display.print("MAC: ");
  display.print(mac[5], HEX);
  display.print(":");
  display.print(mac[4], HEX);
  display.print(":");
  display.print(mac[3], HEX);
  display.print(":");
  display.print(mac[2], HEX);
  display.print(":");
  display.print(mac[1], HEX);
  display.print(":");
  display.println(mac[0], HEX);
  display.display();

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
    display.clearBuffer();
    display.setCursor(0, 0);
    display.print(frame.display_buffer);
    display.display();

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
    return;
  }

  mc.working();

  client.println("*2\r\n$4\r\nLPOP\r\n$15\r\nredink:messages");
  delay(100);

  unsigned int len = client.available();

  if (len == 0) {
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

  client.stop();
}

EConnectionChange attemptConnect(void) {
  //  Start by scanning for networks that match the SSID configured from our 'env.h' file.
  display.clearBuffer();
  display.setCursor(0, 0);
  display.print("Scanning...");
  display.display();

  mc.working();
  unsigned long before = millis();
  int count = WiFi.scanNetworks();
  unsigned long after = millis();

  display.clearBuffer();
  display.setCursor(0, 0);
  display.print("scan time:");
  display.println(after - before);

  // Start by scanning for networks that match the SSID configured from our 'env.h' file.
  if (count < 1) {
    mc.failed();
    display.clearBuffer();
    display.setCursor(0, 0);
    display.println("Disconnected.");
    display.display();
    return EConnectionChange::UnchangedDisconnect;
  }

  mc.ok();

  // Attempt to iterate over the number of wifi networks found, maximizing at a reasonable amount.
  for (unsigned char i = 0; i < 255 && i < count; i++) {
    auto ssid = WiFi.SSID(i);

    // If this network does not match the one from our 'env.h', do nothing.
    if (strcmp(ssid, REDINK_WIFI_SSID) != 0) {
      continue;
    }

    // At this point, we have a matching ssid, attempt to connect.
    display.clearBuffer();
    display.setCursor(0, 0);
    display.print(i);
    display.print(": ");
    display.println(WiFi.SSID(i));
    display.display();

    unsigned char attempt = 0;
    int current_status = WL_IDLE_STATUS;

    // Matching SSID connection loop.
    do {
      mc.working();
      current_status = WiFi.begin(ssid, REDINK_WIFI_PASSWORD);

      if (current_status != WL_CONNECTED) {
        display.clearBuffer();
        display.setCursor(0, 0);
        display.print("Failed on #");
        display.println(attempt);
        display.display();
      } else {
        mc.ok();
        // Here, we have successfully connected, drop out of the connection loop.
        break;
      }

      delay(100);
    } while (current_status != WL_CONNECTED && attempt++ < 10);

    // At this point, we found the matching connection loop and have attempted to connect.
    display.clearBuffer();
    display.setCursor(0, 0);
    if (current_status == WL_CONNECTED) {
      display.println("Connected!");
    } else {
      display.println("Failed Connect");
    }
    display.display();
    return current_status == WL_CONNECTED
      ? EConnectionChange::EstablishedConnection
      : EConnectionChange::LostConnection;
  }

  // If no connection attempts were made (we've exhausted our ssid count w/o finding a match),
  // update the display and bow out.

  mc.failed();
  display.clearBuffer();
  display.setCursor(0, 0);
  display.println("SSID Not Found.");
  display.display();
  return EConnectionChange::UnchangedDisconnect;
}
