#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;
void setup()
{
  Serial.begin(115200);
  if (!bmp.begin())
  {
    Serial.println("BMP180 Sensor not found ! ! !");
    while (1)
    {
    
    }
  }
}

void loop()
{
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.print(" hPa");
  Serial.print("  Temp = ");
  Serial.print(bmp.readTemperature());
  Serial.println("ºC");
  delay(500);
}