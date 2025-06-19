#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

#define windDirection 27
#define windSpeedPin 32

#define SD_CS   13
#define SD_MOSI 15
#define SD_MISO 2
#define SD_SCK  14

volatile uint16_t windCounter = 0;

const char* ssid     = "KT_TPLink";
const char* password = "J@yGsumm!t";

const char* ntpServer = "129.6.15.28";
const long  gmtOffset_sec = 8 * 3600;
const int   daylightOffset_sec = 0;

SPIClass spi = SPIClass(VSPI);
char timeString[25];

unsigned long lastLogMillis = 0;
const unsigned long LOG_INTERVAL = 60000; // fallback in case NTP fails

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("Davis Anemometer");

  pinMode(windSpeedPin, INPUT_PULLUP);
  pinMode(windDirection, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" ✅ Connected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  int ntpTries = 0;
  while (!getLocalTime(&timeinfo) && ntpTries++ < 10) {
    Serial.println("⏳ Waiting for NTP...");
    delay(1000);
  }
  Serial.println(getLocalTime(&timeinfo) ? "✅ Time synced." : "❌ Failed to sync time.");

  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spi)) {
    Serial.println("❌ SD card failed!");
    while (true);
  }
  Serial.println("✅ SD card OK.");

  if (!SD.exists("/DavisAnemo_log.csv")) {
    File f = SD.open("/DavisAnemo_log.csv", FILE_WRITE);
    if (f) {
      f.println("DateTime,Rotations,WindSpeedKPH");
      f.close();
    }
  }
}

void loop() {
  static int lastLoggedMinute = -1;
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {
    if (timeinfo.tm_min != lastLoggedMinute) {
      measureWind();
      lastLoggedMinute = timeinfo.tm_min;
      lastLogMillis = millis(); 
    }
  } else {
    if (millis() - lastLogMillis >= LOG_INTERVAL) {
      measureWind();
      lastLogMillis = millis();
    }
  }

  delay(1000);
}

void initialiseWindCount() {
  windCounter = 0;
}

void IRAM_ATTR incrementWindCount() {
  windCounter++;
}

void measureWind() {

  initialiseWindCount();
  attachInterrupt(digitalPinToInterrupt(windSpeedPin), incrementWindCount, FALLING);

  uint16_t totalWindBearing = 0;
  const uint8_t averageCount = 5;
  for (int i = 0; i < averageCount; i++) {
    totalWindBearing += readWindBearing();
    delay(1000);
  }

  detachInterrupt(digitalPinToInterrupt(windSpeedPin));

  uint16_t rotations = windCounter;
  float windSpeedKPH = (rotations / 1000.0) / (averageCount / 3600.0); // assuming 5 seconds

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(timeString, "1970-01-01 00:00:00");
  }

  Serial.printf("%s | Rotations: %d | Wind Speed: %.2f kph\n", timeString, rotations, windSpeedKPH);
  logToSD(timeString, rotations, windSpeedKPH);
}

void logToSD(const char* timestamp, uint16_t rotations, float windSpeed) {
  File file = SD.open("/DavisAnemo_log.csv", FILE_APPEND);
  if (file) {
    file.printf("%s,%d,%.2f\n", timestamp, rotations, windSpeed);
    file.flush(); 
    file.close();
    Serial.println("✅ Logged to SD");
  } else {
    Serial.println("❌ SD write failed.");
  }
}

uint16_t readWindBearing() {
  uint16_t windReading = analogRead(windDirection);
  return myMap(windReading, 0, 4095, 0, 359);
}

uint16_t myMap(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
