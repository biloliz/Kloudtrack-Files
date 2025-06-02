#include <Wire.h>

// AS5600 I2C address
#define AS5600_ADDRESS 0x36

// AS5600 register addresses
#define RAW_ANGLE_REG 0x0C

unsigned int offsetRawAngle = 0;  // Stores the raw‐angle count at the last calibration
bool      isCalibrated   = false; // Tracks whether we've ever run calibration

void setup() {
  Wire.begin();            // Initialize I2C
  Serial.begin(115200);    // Start Serial at 115200 baud
  // Give the user a hint:
  Serial.println(F("Type \"calibrate\" and press Enter to set the current angle as 0°.\n"));
}

void loop() {
  // 1) Read the raw 12‐bit angle from AS5600:
  unsigned int rawAngle = readAS5600Angle();

  // 2) Check Serial input for the calibration command:
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("calibrate")) {
      // Record the current rawAngle as our zero reference
      offsetRawAngle = rawAngle;
      isCalibrated   = true;
      Serial.println(F(">> Calibrated. Current position is now set to 0°.\n"));
    }
  }

  // 3) Compute a “relative” 12‐bit count (0…4095) after subtracting offset:
  unsigned int relativeRaw;
  if (isCalibrated) {
    int diff = int(rawAngle) - int(offsetRawAngle);
    if (diff < 0) diff += 4096;   // wrap negative into [0..4095]
    relativeRaw = unsigned(diff);
  } else {
    // If not calibrated yet, just use the rawAngle directly:
    relativeRaw = rawAngle;
  }

  // 4) Convert that 0…4095 value into degrees (0.00…360.00):
  float degrees = (relativeRaw / 4096.0) * 360.0;

  // 5) Print both the raw count and the computed degrees:
  Serial.print(F("Raw: "));
  Serial.print(rawAngle);
  if (isCalibrated) {
    Serial.print(F(" (offset "));
    Serial.print(offsetRawAngle);
    Serial.print(F(")"));
  }
  Serial.print(F("  →  "));
  Serial.print(degrees, 2);
  Serial.println(F(" °"));

  delay(100);
}

// Reads the 12-bit raw angle register from the AS5600 over I2C
unsigned int readAS5600Angle() {
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(RAW_ANGLE_REG);     // Point to MSB of RAW ANGLE (0x0C)
  Wire.endTransmission(false);   // Restart when requesting

  Wire.requestFrom(AS5600_ADDRESS, 2);
  while (Wire.available() < 2);  // Wait until 2 bytes arrive

  unsigned int value = Wire.read();  // MSB
  value <<= 8;
  value |= Wire.read();              // LSB
  // Now 'value' is in [0 … 4095]

  return value;
}
