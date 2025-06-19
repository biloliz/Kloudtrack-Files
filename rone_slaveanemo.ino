#include <Wire.h>

#define REED_PIN 2    
#define SAMPLE_INTERVAL 5000  

volatile uint16_t rotationCount = 0;
const float KMH_PER_ROTATION = 1.0;  

float windSpeed = 0;
uint16_t lastRotations = 0;
String windData = "";

void setup() {
  Serial.begin(115200);
  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), countRotation, FALLING);

  Wire.begin(0x08); 
  Wire.onRequest(sendData); 

  Serial.println("🌬️ Wind Sensor Slave Ready (I2C)");
}

void loop() {
  static unsigned long lastSample = 0;
  unsigned long now = millis();

  if (now - lastSample >= SAMPLE_INTERVAL) {
    detachInterrupt(digitalPinToInterrupt(REED_PIN));
    delay(10);
    lastRotations = rotationCount;
    rotationCount = 0;
    attachInterrupt(digitalPinToInterrupt(REED_PIN), countRotation, FALLING);

    float circumference = 0.78; 
    float actualRotations = lastRotations / 4.0;  
    float seconds = SAMPLE_INTERVAL / 1000.0;
    float rotationsPerSecond = actualRotations / seconds;
    windSpeed = (circumference * rotationsPerSecond) * 3.6;

    windData = String(actualRotations) + "," + String(windSpeed, 2);

    Serial.print("Rotations: ");
    Serial.print(actualRotations);
    Serial.print(" | Wind Speed: ");
    Serial.print(windSpeed, 2);
    Serial.println(" km/h");

    lastSample = now;
  }

  delay(10); 
}

void countRotation() {
  rotationCount++;
}

void sendData() {
  Wire.write((const uint8_t*)windData.c_str(), windData.length());
}

