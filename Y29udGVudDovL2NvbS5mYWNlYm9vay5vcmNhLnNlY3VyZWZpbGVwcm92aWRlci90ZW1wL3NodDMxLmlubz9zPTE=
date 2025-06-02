#include <Adafruit_SHT31.h>
Adafruit_SHT31 Sensor = Adafruit_SHT31();
void setup() 
{
  Serial.begin(115200);
      delay(10);
      Serial.println("SHT31 test");
      if (! Sensor.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
      Serial.println("Couldn't find SHT31");
      while (1) delay(1);
      }
}

void loop() 
{

    float t = Sensor.readTemperature();
    float h = Sensor.readHumidity();
    
    if (! isnan(t)) {  // check if 'is not a number'
      Serial.print("Temp *C = "); 
      Serial.println(t);
    }
    else {    
      t = 0.0;
      Serial.println("Failed to read temperature");
    }
    
    if (! isnan(h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); 
      Serial.println(h);
    }
    else { 
      h = 0.0;
      Serial.println("Failed to read humidity");    
    }
      Serial.println();
      delay(500);
}