#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <Ticker.h>
#include <time.h>
#include <simpleDSTadjust.h>
#include <Dusk2Dawn.h>
#include <ESPAsyncTCP.h>
#include "constants.h"

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

const int RS = D2, EN = D3, d4 = D5, d5 = D6, d6 = D7, d7 = D8;   
LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0}; // Central European Time = UTC/GMT +1 hour

Ticker ticker1;
Ticker ticker2;
int32_t tick;
bool readyForNtpUpdate = false;
simpleDSTadjust dstAdjusted(StartRule, EndRule);
Dusk2Dawn dusk2dawn(LAT, LON, 1);

byte backSlashChar[] = {
  0x00,
  0x10,
  0x08,
  0x04,
  0x02,
  0x01,
  0x00,
  0x00
};

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  Serial.print("Connecting to WiFi");

  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DHCP_CLIENTNAME);
  
  WiFi.begin(SSID, PASSWORD);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print(" connected: ");
  Serial.println(WiFi.localIP());

  NBNS.begin(DHCP_CLIENTNAME);
  lcd.begin(16, 2);
  lcd.createChar(0, backSlashChar);
  updateNTP();

  tick = NTP_UPDATE_INTERVAL_SEC;
  ticker1.attach(1, secTicker);
  ticker2.attach(0.3, percentTicker);

  setupAsyncHWinfoClient();
}

void updateNTP() {
  
  configTime(TIMEZONE * 3600, 0, NTP_SERVERS);

  delay(500);
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
}

int lastTick = 0;
char ticks[] = {'|', '/', '-', '\\', '|', '/', '-', '\\'};

void percentTicker() {
  lcd.setCursor(8, 0);
  ++lastTick;
  if(lastTick > 7) {
    lastTick = 0;
  }

  if(ticks[lastTick] == '\\') {
    lcd.write(byte(0));
  } else {
    lcd.print(ticks[lastTick]);
  }
}

void secTicker()
{
  tick--;
  if(tick<=0)
   {
    readyForNtpUpdate = true;
    tick= NTP_UPDATE_INTERVAL_SEC; // Re-arm
   }

  char *dstAbbrev;
  time_t t = dstAdjusted.time(&dstAbbrev);
  struct tm *timeinfo = localtime (&t);

  lcd.setCursor(0, 0);
  lcd.print("ESP8266 /");

  lcd.setCursor(0, 1);
  lcd.print(timeinfo->tm_year + 1900);
  lcd.print(".");
  lcd.printf("%02d", timeinfo->tm_mon + 1);
  lcd.print(".");
  lcd.printf("%02d", timeinfo->tm_mday);

  lcd.setCursor(11, 1);
  lcd.printf("%02d", timeinfo->tm_hour);
  if(timeinfo->tm_sec % 2 == 0) {
    lcd.print(":");
  } else {
    lcd.print(" ");
  }
  lcd.printf("%02d", timeinfo->tm_min);

  
}

void loop() {
  if(readyForNtpUpdate)
  {
    readyForNtpUpdate = false;
    updateNTP();
  }
}
