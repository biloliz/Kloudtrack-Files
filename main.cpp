#include <Wire.h>
#include <AS5600.h>
#include <BH1750.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SHT31.h>

AS5600 as5600;
BH1750 lightMeter;
Adafruit_BMP085 bmp;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

const int uvSensorPin = 32; 
void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!as5600.begin()) {
    Serial.println("AS5600 not detected.");
  }

  if (!lightMeter.begin()) {
    Serial.println("BH1750 not detected.");
  }

  if (!bmp.begin()) {
    Serial.println("BMP180 not detected.");
  }

  if (!sht31.begin(0x44)) { 
    Serial.println("SHT30 not detected.");
  }
}

void loop() {
  int angle = as5600.getAngle();
  Serial.print("AS5600 Angle: ");
  Serial.print(angle);
  Serial.println(" degrees");

  float lux = lightMeter.readLightLevel();
  Serial.print("BH1750 Light Level: ");
  Serial.print(lux);
  Serial.println(" lux");

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure();
  Serial.print("BMP180 Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("BMP180 Pressure: ");
  Serial.print(pressure);
  Serial.println(" Pa");

  float shtTemp = sht31.readTemperature();
  float humidity = sht31.readHumidity();
  Serial.print("SHT30 Temperature: ");
  Serial.print(shtTemp);
  Serial.println(" *C");
  Serial.print("SHT30 Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  int uvRaw = analogRead(uvSensorPin);
  float uvVoltage = (uvRaw / 4095.0) * 3.3;
  Serial.print("GUVA-S12SD UV Raw: ");
  Serial.print(uvRaw);
  Serial.print(" | Voltage: ");
  Serial.print(uvVoltage, 3);
  Serial.println(" V");

  Serial.println("-----------------------------");
  delay(500);
}
