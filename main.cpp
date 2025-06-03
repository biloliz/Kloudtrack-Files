#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SHT31.h>

#define SEALEVELPRESSURE_HPA 1013.25
#define AS5600_ADDRESS 0x36
#define RAW_ANGLE_REG  0x0C

// Sensor objects
BH1750 lightSensor;
Adafruit_BME280 bme;
Adafruit_BMP085 bmp;
Adafruit_SHT31 sht;

bool bhFound = false, bmeFound = false, bmpFound = false, shtFound = false;
unsigned int offsetRawAngle = 0;
bool isCalibrated = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize I2C sensors
  bhFound  = lightSensor.begin();
  bmpFound = bmp.begin();
 // bmeFound = bme.begin(0x76);
  shtFound = sht.begin(0x44);

  Serial.println("\n--- Sensor Check ---");
  Serial.println(bhFound  ? "BH1750: OK" : "BH1750: Not Found");
  Serial.println(bmpFound ? "BMP180: OK" : "BMP180: Not Found");
  Serial.println(bmeFound ? "BME280: OK" : "BME280: Not Found");
  Serial.println(shtFound ? "SHT31: OK" : "SHT31: Not Found");
  Serial.println("AS5600: Will test below\n");
}

void loop() {
  // Read and show sensors
  if (bhFound) {
    float lux = lightSensor.readLightLevel();
    Serial.printf("Light: %.2f lx\n", lux);
  }

  if (bmpFound) {
    Serial.printf("BMP180 Temp: %.2f °C\nPressure: %.2f hPa\n",
                  bmp.readTemperature(), bmp.readPressure() / 100.0);
  }

  if (shtFound) {
    float t = sht.readTemperature();
    float h = sht.readHumidity();
    if (!isnan(t)) Serial.printf("SHT31 Temp: %.2f °C\n", t);
    if (!isnan(h)) Serial.printf("SHT31 Humidity: %.2f %%\n", h);
  }

//  if (bmeFound) {
  //  Serial.printf("BME280 Temp: %.2f °C, Humidity: %.2f %%, Pressure: %.2f hPa\n",
 //                 bme.readTemperature(),
 /*                 bme.readHumidity(),
                  bme.readPressure() / 100.0F);
  }
*/
  // --- AS5600 ---
  readAS5600AngleInDegrees();

  Serial.println("----------------------------\n");
  delay(3000);
}

void readAS5600AngleInDegrees() {
  unsigned int rawAngle = readAS5600Raw();

  if (rawAngle == 0xFFFF) {
    Serial.println("AS5600: Not responding");
    return;
  }

  // Calibration via serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("calibrate")) {
      offsetRawAngle = rawAngle;
      isCalibrated = true;
      Serial.println("AS5600: Calibrated to 0°");
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

  float degrees = (relativeRaw / 4096.0) * 360.0;
  Serial.printf("AS5600 Angle: %.2f °\n", degrees);
}

unsigned int readAS5600Raw() {
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(RAW_ANGLE_REG);
  if (Wire.endTransmission(false) != 0) return 0xFFFF;

  Wire.requestFrom(AS5600_ADDRESS, 2);
  if (Wire.available() < 2) return 0xFFFF;

  unsigned int value = Wire.read() << 8 | Wire.read();
  return value & 0x0FFF; // Keep only the 12-bit value
}
