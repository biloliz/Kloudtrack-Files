#include <Wire.h>

// AS5600
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_REG 0x0C
unsigned int offsetRawAngle = 0;
bool isCalibrated = false;

// BH1750
#include <BH1750.h>
BH1750 lightSensor;

// BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

// SHT30
#include <Adafruit_SHT31.h>
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // AS5600
  Serial.println(F("AS5600 Ready. Type \"calibrate\" to set zero angle."));

  // BH1750
  if (!lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 not detected."));
  }

  // BME280
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not detected. Check wiring.");
    while (1);
  }

  // SHT31
  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 not detected. Check wiring.");
    while (1);
  }
}

void loop() {
  // AS5600
  unsigned int rawAngle = readAS5600Angle();
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("calibrate")) {
      offsetRawAngle = rawAngle;
      isCalibrated = true;
      Serial.println(">> Calibrated. Current position is now set to 0°.");
    }
  }

  unsigned int relativeRaw;
  if (isCalibrated) {
    int diff = int(rawAngle) - int(offsetRawAngle);
    if (diff < 0) diff += 4096;
    relativeRaw = unsigned(diff);
  } else {
    relativeRaw = rawAngle;
  }

  float angleDegrees = (relativeRaw / 4096.0) * 360.0;

  Serial.print("AS5600 Raw: ");
  Serial.print(rawAngle);
  Serial.print(" → ");
  Serial.print(angleDegrees, 2);
  Serial.println(" °");

  // BH1750
  float lux = lightSensor.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  // BME280
  Serial.print("BME280 Temp: ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure: ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Altitude: ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  // SHT30
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (!isnan(t)) {
    Serial.print("SHT31 Temp: ");
    Serial.println(t);
  } else {
    Serial.println("Failed to read SHT31 temperature");
  }

  if (!isnan(h)) {
    Serial.print("SHT31 Humidity: ");
    Serial.println(h);
  } else {
    Serial.println("Failed to read SHT31 humidity");
  }

  // GUVA
  float sensorValue = analogRead(32);
  float sensorVoltage = sensorValue / 4095.0 * 3.3;
  Serial.print("Analog Sensor Value: ");
  Serial.println(sensorValue);
  Serial.print("Analog Sensor Voltage: ");
  Serial.print(sensorVoltage);
  Serial.println(" V");

  Serial.println("-----------------------------\n");
  delay(500);
}

unsigned int readAS5600Angle() {
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(RAW_ANGLE_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDRESS, 2);
  while (Wire.available() < 2);
  unsigned int value = Wire.read();
  value <<= 8;
  value |= Wire.read();
  return value;
}
