#include <SPI.h>
#include <WiFiNINA.h>
#include "Adafruit_ThinkInk.h"

#include "board-layout.h"
#include "env.h"

#include "mc.h"

const unsigned int MIN_MESSAGE_DISPLAY_WAIT = 3000;
const unsigned int MIN_NOMESSAGE_DISPLAY_WAIT = 10000;
const unsigned int MIN_RECONNECT_WAIT = 10000;

netdisplay::Mc mc(DOTSTAR_DATA_PIN, DOTSTAR_CLOCK_PIN, DOTSTAR_BGR);

WiFiServer server(8081);

ThinkInk_290_Mono_M06 display(
  DISPLAY_DC_PIN,
  DISPLAY_RESET_PIN,
  DISPLAY_CS_PIN,
  DISPLAY_SRAM_CS_PIN,
  DISPLAY_BUSY_PIN
);

struct TFrameInfo {
  unsigned long last_reconnect;
  int last_connection_status;
  unsigned char last_client_status;
  bool server_started;

  unsigned long last_display_time;
  unsigned long last_client_message_time;
  char last_client_message [255];
} frame = {
  last_reconnect: 0,
  last_connection_status: WL_IDLE_STATUS,
  last_client_status: 0,
  server_started: false,

  last_display_time: 0,

  last_client_message_time: 0,
  last_client_message: {'\0'},
};

enum EConnectionChange {
  UnchangedDisconnect = 0,
  UnchangedConnected = 1,
  EstablishedConnection = 2,
  LostConnection = 3,
};

void setup(void) {
  Serial.begin(9600);

  for (unsigned int i = 0; i < 500; i++) {
    mc.booting(i);
    delay(10);
  }

  mc.ok();
  delay(500);

  mc.working();
  display.begin(THINKINK_MONO);

  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);

  display.clearBuffer();
  display.setCursor(0, 0);
  display.print("Preparing WiFi...");
  display.display();

  mc.ok();
  delay(500);

  mc.working();
  WiFi.setPins(WIFI_CS_PIN, WIFI_BUSY_PIN, WIFI_RESET_PIN, WIFI_GPIO_PIN, &SPI);

  mc.ok();
  delay(500);

  mc.working();
  while (WiFi.status() == WL_NO_MODULE) {
    mc.failed();
    delay(1000);
  }

  mc.ok();
  delay(500);

  mc.working();
  
  byte mac[6];
  String fv = WiFi.firmwareVersion();
  WiFi.macAddress(mac);

  display.clearBuffer();
  display.setCursor(0, 0);
  display.println(fv);
  display.print("MAC: ");
  display.print(mac[5],HEX);
  display.print(":");
  display.print(mac[4],HEX);
  display.print(":");
  display.print(mac[3],HEX);
  display.print(":");
  display.print(mac[2],HEX);
  display.print(":");
  display.print(mac[1],HEX);
  display.print(":");
  display.println(mac[0],HEX);
  display.display();

  mc.ok();
  delay(500);
}

void loop(void) {
  unsigned long now = millis();

  bool reconnect_time = frame.last_reconnect == 0 || now - frame.last_reconnect > MIN_RECONNECT_WAIT;

  if (reconnect_time && frame.last_connection_status != WL_CONNECTED) {
    EConnectionChange result = attemptConnect();
    frame.last_reconnect = now;

    if (result == EConnectionChange::EstablishedConnection && !frame.server_started) {
      server.begin();
      frame.server_started = true;
    }
  }

  // If we have received a message, and it was debounced long enough, display the message.
  if (frame.last_client_message_time > 0 && now - frame.last_client_message_time > MIN_MESSAGE_DISPLAY_WAIT) {
    display.clearBuffer();
    display.setCursor(0, 0);
    display.print(frame.last_client_message);
    display.display();

    // After displaying our message, reset the time and clear out our message.
    frame.last_client_message_time = 0;
    frame.last_display_time = millis();
    memset(frame.last_client_message, '\0', 255);
  }

  // Update our connection status
  frame.last_connection_status =  WiFi.status();

  if (frame.last_connection_status != WL_CONNECTED || frame.server_started == false) {
    return;
  }

  WiFiClient client = server.available();

  if (!client) {
    // If there is no current connection, and it has been some time since we displayed a message, clear it and
    // reset our connection status to off.
    if (frame.last_display_time > 0 && millis() - frame.last_display_time > MIN_NOMESSAGE_DISPLAY_WAIT) {
      if (frame.last_client_status == 1) {
        display.clearBuffer();
        display.setCursor(0, 0);
        display.print("No Client");
        display.display();
      }

      frame.last_client_status = 0;
      frame.last_display_time = 0;
    }

    return;
  }

  // Attempt to receive up to some number of bytes from our clientAttwempt.
  char framebuffer [255] = {'\0'};

  bool method = false;
  bool path = false;

  int len = client.available();
  int idx = 0;
  while (len > 0 && idx < 255) {
    framebuffer[idx] = client.read();

    // If we have received a valid http message, take note and clear out our current
    // framebuffer, starting back at 0;
    if (idx == 2 && strcmp(framebuffer, "GET") == 0) {
      method = true;

      // Space delimeter should be next, just skip it.
      client.read();

      // Reset our current frame.
      memset(framebuffer, '\0', 255);
      idx = 0;
      
      // We have technically read 2 bytes this iteration.
      len = len - 2;
      continue;
    }

    if (method && !path && framebuffer[idx] == ' ') {
      path = true;
      memcpy(frame.last_client_message, framebuffer, idx);
    }

    idx++;
    len--;
  }

  if (method == false || path == false) {
    memcpy(frame.last_client_message, "Bad", 3);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Content-Length: 2");
  client.println("Connection: close");
  client.println();
  client.println("ok");

  delay(10);

  // Update our state so we know on the next iteration that:
  // 1. we had a client
  // 2. the message was received at this point in time.
  frame.last_client_status = 1;
  frame.last_client_message_time = millis();

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

  for (unsigned char i = 0; i < 255 && i < count; i++) {
    auto ssid = WiFi.SSID(i);

    // If this network does not match the one from our 'env.h', do nothing.
    if (strcmp(ssid, JOY_WIFI_SSID) != 0) {
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
      current_status = WiFi.begin(ssid, JOY_WIFI_PASSWORD);

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
