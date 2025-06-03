#include <Wire.h>
#include "Adafruit_SHT31.h"

Adafruit_SHT31 sht = Adafruit_SHT31(); 

#define TCA_ADDR 0x70  
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);  
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  for (uint8_t ch = 0; ch < 3; ch++) {
    tcaSelect(ch);
    delay(10); 
    if (!sht.begin(0x44)) {
      Serial.print("SHT30 not found on channel ");
      Serial.println(ch);
    } else {
      Serial.print("SHT30 detected on channel ");
      Serial.println(ch);
    }
  }
}

void loop() {
  for (uint8_t ch = 0; ch < 3; ch++) {
    tcaSelect(ch);
    delay(10);

    float temp = sht.readTemperature();
    float hum = sht.readHumidity();

    Serial.print("CH ");
    Serial.print(ch);
    if (!isnan(temp) && !isnan(hum)) {
      Serial.print(" | Temp: ");
      Serial.print(temp);
      Serial.print(" °C, Hum: ");
      Serial.print(hum);
      Serial.println(" %");
    } else {
      Serial.println(" | Failed to read data");
    }

    delay(1000);
  }

  Serial.println("-------------------------");
  delay(2000);
}
