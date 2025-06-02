#include <Wire.h>
#include <BH1750.h>

BH1750 Sensor;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Sensor.begin();
  Serial.println(F("BH1750 Test Begin"));

}

void loop() {
  float lux = Sensor.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(500);

}
