#include <SPI.h>
#include <WiFiNINA.h>
#include "Adafruit_ThinkInk.h"

// Configure the pins used for the ESP32 connection
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    A2   // Chip select pin
#define ESP32_RESETN  A4   // Reset pin
#define SPIWIFI_ACK   A3   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

#define EPD_CS      7
#define EPD_DC      9
#define SRAM_CS     10
#define EPD_RESET   11 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    12 // can set to -1 to not use a pin (will wait a fixed delay)

// 2.9" Monochrome displays with 296x128 pixels and UC8151D chipset
ThinkInk_290_Mono_M06 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);


void setup() {
  Serial.begin(9600);
  display.begin(THINKINK_MONO);
  
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

  while (WiFi.status() == WL_NO_MODULE) {
    delay(1000);
  }
  
  String fv = WiFi.firmwareVersion();

  display.clearBuffer();
  display.setTextSize(3);
  display.setCursor(0, (display.height() - 24)/2);
  display.setTextColor(EPD_BLACK);
  display.print(fv);
  display.display();
}

void loop() {
  listNetworks();
  delay(3000);
}

void listNetworks() {
  int numSsid = WiFi.scanNetworks();
  
  if (numSsid == -1) {
    display.clearBuffer();
    display.setTextSize(3);
    display.setCursor(0, (display.height() - 24)/2);
    display.setTextColor(EPD_BLACK);
    display.print("disconnected");
    display.display();
    return;
  }

  for (int thisNet = 0; thisNet < 1; thisNet++) {
    display.clearBuffer();
    display.setTextSize(3);
    display.setCursor(0, (display.height() - 24)/2);
    display.setTextColor(EPD_BLACK);
    display.print("W:");
    display.print(WiFi.SSID(thisNet));
    display.display();
  }
}
