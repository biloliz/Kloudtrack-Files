#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// Wi-Fi credentials
const char* ssid     = "KT_TPLink";
const char* password = "J@yGsumm!t";

// NTP settings
const char* ntpServer = "129.6.15.28";
const long  gmtOffset_sec = 8 * 3600;  // GMT+8
const int   daylightOffset_sec = 0;

#define SD_CS   13
#define SD_MOSI 15
#define SD_MISO 2
#define SD_SCK  14

SPIClass spi = SPIClass(VSPI);
char timeString[25];

#define SLAVE_ADDR 0x08

void setup() {
  Serial.begin(115200);
  Wire.begin();  // I2C Master

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" ✅ Connected!");

  // Get NTP time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  int ntpTries = 0;
  while (!getLocalTime(&timeinfo) && ntpTries++ < 10) {
    Serial.println("⏳ Waiting for NTP...");
    delay(1000);
  }
  Serial.println(getLocalTime(&timeinfo) ? "✅ Time synced." : "❌ Failed to sync time.");

  // SD card
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spi)) {
    Serial.println("❌ SD card failed!");
    while (true);
  }
  Serial.println("✅ SD card OK.");

  if (!SD.exists("/KTAnemo_log.csv")) {
    File f = SD.open("/KTAnemo_log.csv", FILE_WRITE);
    if (f) {
      f.println("DateTime,Rotations,WindSpeedKPH");
      f.close();
    }
  }
}

void loop() {
  static unsigned long lastRead = 0;
  const unsigned long readInterval = 60000;

  if (millis() - lastRead >= readInterval) {
    char buffer[32];
    int i = 0;

    Wire.requestFrom(SLAVE_ADDR, 32); // Request up to 32 bytes
    while (Wire.available() && i < 31) {
      char c = Wire.read();
      if (c == '\n') break;
      buffer[i++] = c;
    }
    buffer[i] = '\0'; // null terminate

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
      strcpy(timeString, "1970-01-01 00:00:00");
    }

    Serial.print("📥 Received: ");
    Serial.println(buffer);

    logToSD(String(timeString), String(buffer));
    lastRead = millis();
  }

  delay(100);
}

void logToSD(const String& timestamp, const String& dataLine) {
  File file = SD.open("/KTAnemo_log.csv", FILE_APPEND);
  if (file) {
    file.print(timestamp);
    file.print(",");
    file.println(dataLine);
    file.close();
    Serial.println("✅ Logged to SD");
  } else {
    Serial.println("❌ SD write failed.");
  }
}