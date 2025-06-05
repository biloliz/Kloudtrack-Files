#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SHT31.h>
#include "AS5600.h"            // Rob Tillaart’s AS5600 library

// ———— Constants ————
#define MUX_ADDRESS      0x70    // I²C address of HW-617 (TCA9548A)
#define BME280_ADDRESS   0x76    // Change to 0x77 if your BME280 modules use that address

// ———— Sensor Objects ————
BH1750          lightMeter;       // BH1750 light sensor
Adafruit_BME280 bme1;             // BME280 #1 on MUX channel 2
Adafruit_BME280 bme2;             // BME280 #2 on MUX channel 3
Adafruit_BME280 bme3;             // BME280 #3 on MUX channel 4
Adafruit_SHT31  sht30 = Adafruit_SHT31();  // SHT30 temperature/humidity (needs offset)
AS5600          as5600;           // AS5600 magnetic rotation meter

// GUVA-S12SD (UV sensor) → analog pin 32
const int UV_PIN = 32;

// ———— Calibration Offsets for SHT30 ————
const float tempOffset  = -3.5;   // (T_ref − T_sht) measured
const float humidOffset = +8.7;   // (RH_ref − RH_sht) measured

// ———— AS5600 Reference Angle ————
float angleOffsetDeg = 0.0;       // in degrees; set when user types 'C'

// ———— Select one channel on the HW-617 multiplexer (TCA9548A) ————
void selectMuxChannel(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(MUX_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// ———— Disable all MUX channels ————
void disableAllMux() {
  Wire.beginTransmission(MUX_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  // ESP32 default I²C: SDA = GPIO21, SCL = GPIO22
  Wire.begin(21, 22);
  delay(100);

  // ———— Initialize BH1750 (light sensor) ————
  if (lightMeter.begin()) {
    Serial.println("[OK] BH1750 initialized.");
  } else {
    Serial.println("[ERR] BH1750 failed to initialize.");
  }

  // ———— Initialize SHT30 (temp/humidity) ————
  // Default I²C address: 0x44. Change to 0x45 if ADDR pin is high.
  if (sht30.begin(0x44)) {
    Serial.println("[OK] SHT30 initialized.");
  } else {
    Serial.println("[ERR] SHT30 failed to initialize!");
  }

  // ———— Initialize AS5600 (magnetic rotation meter) ————
  if (as5600.begin()) {
    Serial.print("[OK] AS5600 initialized. I²C addr: 0x");
    Serial.println(as5600.getAddress(), HEX);
  } else {
    Serial.println("[WARN] AS5600 not detected. Angle readings will be zero.");
  }

  // ———— Initialize each BME280 on its own MUX channel ————
  // BME1 → channel 2
  selectMuxChannel(2);
  if (bme1.begin(BME280_ADDRESS)) {
    Serial.println("[OK] BME280 #1 initialized on MUX channel 2");
  } else {
    Serial.println("[ERR] BME280 #1 failed on MUX channel 2");
  }

  // BME2 → channel 3
  selectMuxChannel(3);
  if (bme2.begin(BME280_ADDRESS)) {
    Serial.println("[OK] BME280 #2 initialized on MUX channel 3");
  } else {
    Serial.println("[ERR] BME280 #2 failed on MUX channel 3");
  }

  // BME3 → channel 4
  selectMuxChannel(4);
  if (bme3.begin(BME280_ADDRESS)) {
    Serial.println("[OK] BME280 #3 initialized on MUX channel 4");
  } else {
    Serial.println("[ERR] BME280 #3 failed on MUX channel 4");
  }

  // Deselect all MUX channels
  disableAllMux();

  Serial.println();
  Serial.println("Type 'C' in Serial Monitor to set current AS5600 angle as 0° reference.");
  Serial.println("--------------------------------------------------------------");
  Serial.println("---- Starting sensor read loop ----");
  delay(200);
}

void loop() {
  // ———— Check for Serial input to set reference angle ————
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'C' || c == 'c') {
      int32_t cumPos = as5600.getCumulativePosition();     // signed cumulative position
      int32_t raw12  = cumPos % 4096;                      // bottom 12 bits
      if (raw12 < 0) raw12 += 4096;                        // wrap into [0..4095]
      float angleNow = raw12 * (360.0 / 4096.0);           // degrees
      angleOffsetDeg = angleNow;
      Serial.print(">>> Reference angle set to: ");
      Serial.print(angleOffsetDeg, 2);
      Serial.println(" °");
      Serial.println("--------------------------------------------------------------");
    }
  }

  // ———— Deselect all MUX channels so other I²C devices see the main bus ————
  disableAllMux();

  // ———— Read BH1750 (light) ————
  float lux = lightMeter.readLightLevel(); // in lux

  // ———— Read SHT30 (raw temperature, raw humidity) ————
  float rawTemp = sht30.readTemperature(); // °C
  float rawHum  = sht30.readHumidity();    // %

  float shtTempCal = NAN, shtHumCal = NAN;
  if (!isnan(rawTemp) && !isnan(rawHum)) {
    shtTempCal = rawTemp + tempOffset;
    shtHumCal  = rawHum  + humidOffset;
    if (shtHumCal < 0.0)   shtHumCal = 0.0;
    if (shtHumCal > 100.0) shtHumCal = 100.0;
  }

  // ———— Read GUVA-S12SD (UV) ————
  int   uvRaw     = analogRead(UV_PIN);
  float uvVoltage = uvRaw * (3.3 / 4095.0);  // ADC 0–4095 → 0–3.3 V
  float uvIntensity = 0.0;
  if (uvVoltage >
