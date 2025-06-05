#include <Wire.h>
#include <Adafruit_BME280.h>

#define TCA_ADDR 0x70
Adafruit_BME280 bme; // I2C

void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // Adjust SDA/SCL if needed

  for (uint8_t ch = 2; ch < 5; ch++) {
    tcaSelect(ch);
    delay(10);

    if (!bme.begin(0x76)) { // Default I2C address of BME280 (can be 0x76 or 0x77)
      Serial.print("BME280 not found on channel ");
      Serial.println(ch);
    } else {
      Serial.print("BME280 detected on channel ");
      Serial.println(ch);
    }
  }
}

void loop() {
  for (uint8_t ch = 2; ch < 5; ch++) {
    tcaSelect(ch);
    delay(10);

    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;

    Serial.print("CH ");
    Serial.print(ch);
    if (!isnan(temp) && !isnan(hum) && !isnan(pres)) {
      Serial.print(" | Temp: ");
      Serial.print(temp);
      Serial.print(" °C, Hum: ");
      Serial.print(hum);
      Serial.print(" %, Pressure: ");
      Serial.print(pres);
      Serial.println(" hPa");
    } else {
      Serial.println(" | Failed to read data");
    }

    delay(500);
  }

  Serial.println("-------------------------");
  delay(1000);
}
