#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <BH1750.h>
#include <Adafruit_SHT31.h>

// BMP180
Adafruit_BMP085 bmp;

// BH1750
BH1750 lightSensor;

// SHT31
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// AS5600
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_REG 0x0C
unsigned int offsetRawAngle = 0;
bool isCalibrated = false;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(10);

  // BMP180
  if (!bmp.begin()) {
    Serial.println("BMP180 Sensor not found!");
    while (1);
  }

  // BH1750
  if (!lightSensor.begin()) {
    Serial.println("BH1750 Sensor not found!");
    while (1);
  }

  // SHT31
  Serial.println("Initializing SHT31...");
  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT31!");
    while (1);
  }

  Serial.println(F("All sensors initialized. Type \"calibrate\" to zero AS5600 angle."));
}

void loop() {
  // --- BMP180 ---
  Serial.print("Pressure: ");
  Serial.print(bmp.readPressure() / 100.0);
  Serial.print(" hPa | Temp (BMP180): ");
  Serial.print(bmp.readTemperature());
  Serial.print(" °C");

  // --- BH1750 ---
  float lux = lightSensor.readLightLevel();
  Serial.print(" | Light: ");
  Serial.print(lux);
  Serial.print(" lx");

  // --- SHT31 ---
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  Serial.print(" | Temp (SHT31): ");
  Serial.print(isnan(t) ? 0.0 : t);
  Serial.print(" °C | Hum: ");
  Serial.print(isnan(h) ? 0.0 : h);
  Serial.print(" %");

  // --- AS5600 ---
  unsigned int rawAngle = readAS5600Angle();
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("calibrate")) {
      offsetRawAngle = rawAngle;
      isCalibrated = true;
      Serial.println("\n>> Calibrated. Current position is now set to 0°.\n");
    }
  }

  unsigned int relativeRaw = isCalibrated
    ? ((int(rawAngle) - int(offsetRawAngle) + 4096) % 4096)
    : rawAngle;

  float degrees = (relativeRaw / 4096.0) * 360.0;
  Serial.print(" | Angle: ");
  Serial.print(degrees, 2);
  Serial.print(" °");

  // --- Analog Voltage (A32) ---
  float sensorValue = analogRead(32);
  float sensorVoltage = sensorValue / 4095.0 * 3.3;
  Serial.print(" | A32: ");
  Serial.print(sensorVoltage, 2);
  Serial.println(" V");

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
