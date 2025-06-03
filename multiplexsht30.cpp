#include <Wire.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht0 = Adafruit_SHT31();  // Direct - 0x44
Adafruit_SHT31 sht1 = Adafruit_SHT31();  // Direct - 0x45
Adafruit_SHT31 sht2 = Adafruit_SHT31();  // Via TCA9548A - 0x44

#define TCA9548A_ADDR 0x70
#define TCA_CHANNEL_FOR_SHT2 0  // Channel for third SHT30

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Sensor 1 (0x44) - direct
  if (!sht0.begin(0x44)) {
    Serial.println("SHT30 #0 not found at 0x44");
  } else {
    Serial.println("SHT30 #0 OK at 0x44");
  }

  // Sensor 2 (0x45) - direct
  if (!sht1.begin(0x45)) {
    Serial.println("SHT30 #1 not found at 0x45");
  } else {
    Serial.println("SHT30 #1 OK at 0x45");
  }

  // Sensor 3 (0x44 via TCA)
  tcaSelect(TCA_CHANNEL_FOR_SHT2);
  if (!sht2.begin(0x44)) {
    Serial.println("SHT30 #2 not found at 0x44 via TCA");
  } else {
    Serial.println("SHT30 #2 OK at 0x44 via TCA");
  }
}

void loop() {
  // Sensor 1
  float t0 = sht0.readTemperature();
  float h0 = sht0.readHumidity();
  Serial.print("[0x44] Temp: "); Serial.print(t0); Serial.print(" °C, Hum: "); Serial.println(h0);

  // Sensor 2
  float t1 = sht1.readTemperature();
  float h1 = sht1.readHumidity();
  Serial.print("[0x45] Temp: "); Serial.print(t1); Serial.print(" °C, Hum: "); Serial.println(h1);

  // Sensor 3 via TCA
  tcaSelect(TCA_CHANNEL_FOR_SHT2);
  float t2 = sht2.readTemperature();
  float h2 = sht2.readHumidity();
  Serial.print("[MUX-0x44] Temp: "); Serial.print(t2); Serial.print(" °C, Hum: "); Serial.println(h2);

  Serial.println("-----");
  delay(2000);
}
