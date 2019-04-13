#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESPAsyncTCP.h>
#include "constants.h"

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.print("Connecting to WiFi");

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DHCP_CLIENTNAME);
  
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print(" connected: ");
  Serial.println(WiFi.localIP());

  NBNS.begin(DHCP_CLIENTNAME);
  lcd.begin(16, 2);

  setupAsyncHWinfoClient();
}

void loop() {
  //
}
