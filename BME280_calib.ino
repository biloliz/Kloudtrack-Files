#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// Wi-Fi credentials
const char* ssid     = "KT_TPLink";
const char* password = "J@yGsumm!t";

// SD card SPI pin config
#define SD_CS     13
#define SD_MOSI   15
#define SD_MISO   2
#define SD_SCK    14

SPIClass spi = SPIClass(VSPI);  // VSPI for ESP32

// I2C multiplexer address
#define TCA_ADDR 0x70

// Create BME280 instances
Adafruit_BME280 bme[3];  // bme[0] on channel 2, bme[1] on 3, bme[2] on 4
bool bmeReady[3] = {false, false, false};

// I2C multiplexer channels for the BME280 sensors
const uint8_t tcaChannels[3] = {2, 3, 4};

// NTP time settings
const char* ntpServer = "129.6.15.28";
const long  gmtOffset_sec = 8 * 3600;
const int   daylightOffset_sec = 0;

char timeString[25];  // For timestamp formatting

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
  delay(100);  // Let I2C settle
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);  // ESP32 I2C pins

  // WiFi connection
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int wifiRetries = 0;
  while (WiFi.status() != WL_CONNECTED && wifiRetries++ < 20) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(WiFi.status() == WL_CONNECTED ? " ✅ Connected!" : " ❌ Failed to connect to WiFi.");

  // NTP time sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  int ntpRetries = 0;
  while (!getLocalTime(&timeinfo) && ntpRetries++ < 10) {
    Serial.println("⏳ Waiting for NTP time...");
    delay(1000);
  }
  Serial.println(getLocalTime(&timeinfo) ? "✅ Time synchronized." : "❌ Failed to synchronize time.");

  // Scan TCA channels and init BME280
  Serial.println("🔌 Initializing BME280 sensors via multiplexer...");
  for (int i = 0; i < 3; i++) {
    tcaSelect(tcaChannels[i]);
    if (bme[i].begin(0x76) || bme[i].begin(0x77)) {
      bmeReady[i] = true;
      Serial.printf("✅ BME280 #%d initialized on TCA channel %d\n", i + 1, tcaChannels[i]);
    } else {
      Serial.printf("❌ BME280 #%d NOT found on TCA channel %d\n", i + 1, tcaChannels[i]);
    }
  }

  // Init SD card
  Serial.println("🔌 Initializing SD card...");
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spi)) {
    Serial.println("❌ SD card initialization failed!");
  } else {
    Serial.println("✅ SD card initialized.");
    if (!SD.exists("/log.csv")) {
      File file = SD.open("/log.csv", FILE_WRITE);
      if (file) {
        // Header row
        file.print("DateTime,");
        for (int i = 1; i <= 3; i++) {
          file.print("bme" + String(i) + "_Pressure,");
          file.print("bme" + String(i) + "_Humidity,");
          file.print("bme" + String(i) + "_Temperature");
          if (i < 3) file.print(",");
        }
        file.println();
        file.close();
        Serial.println("📄 Header written to log.csv");
      }
    }
  }
}

void loop() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(timeString, "1970-01-01 00:00:00");
    Serial.println("⚠️ Time not available.");
  }

  Serial.printf("🕒 Timestamp: %s\n", timeString);
  Serial.println("🔍 Logging BME280 data...");

  float pressure[3] = {NAN, NAN, NAN};
  float humidity[3] = {NAN, NAN, NAN};
  float temperature[3] = {NAN, NAN, NAN};

  for (int i = 0; i < 3; i++) {
    tcaSelect(tcaChannels[i]);
    if (bmeReady[i]) {
      pressure[i] = bme[i].readPressure();
      humidity[i] = bme[i].readHumidity();
      temperature[i] = bme[i].readTemperature();

      Serial.printf("✅ BME%d (TCA channel %d) - P: %.2f Pa, H: %.2f%%, T: %.2f°C\n",
                    i + 1, tcaChannels[i], pressure[i], humidity[i], temperature[i]);
    } else {
      Serial.printf("❌ BME%d not ready (NAN values used).\n", i + 1);
    }
  }

  // Log to CSV
  File file = SD.open("/log.csv", FILE_APPEND);
  if (file) {
    file.print(timeString);
    file.print(",");
    for (int i = 0; i < 3; i++) {
      file.printf("%.2f,%.2f,%.2f", pressure[i], humidity[i], temperature[i]);
      if (i < 2) file.print(",");
    }
    file.println();
    file.close();
    Serial.println("💾 Data logged to SD card.");
  } else {
    Serial.println("❌ Failed to open log file.");
  }

  delay(60000);  // Log every 60 seconds
}
