// BLE MIDI example for nRF52840
#include <bluefruit.h>

#include <Wire.h>
#include <LSM6DS3.h>

// Create the BLE MIDI service
BLEMidi midi;

// Create the IMU object (I2C address 0x6A is default for XIAO nRF52840 Sense)
LSM6DS3 imu(I2C_MODE, 0x6A);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Bluefruit.begin();
  Bluefruit.setName("XIAO-BLE");

  // Start the BLE MIDI service
  midi.begin();

  // Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addService(midi);
  Bluefruit.Advertising.start(0); // Advertise forever

  // Initialize IMU
  if (imu.begin() != 0)
  {
    Serial.println("Failed to initialize LSM6DS3 IMU!");
    while (1)
      delay(1000);
  }
  Serial.println("LSM6DS3 IMU initialized.");
}

void loop()
{
  static uint32_t lastMillis = 0;
  static bool wasConnected = false;
  bool isConnected = Bluefruit.connected();
  if (isConnected && !wasConnected)
  {
    Serial.println("[BLE] Connected!");
  }
  wasConnected = isConnected;

  // Send IMU pitch and roll as MIDI CC 21-22 every 50ms with smoothing
  static float smooth_ax = 0, smooth_ay = 0, smooth_az = 0;
  static float smooth_pitch = 0, smooth_roll = 0;
  static uint8_t lastValPitch = 255, lastValRoll = 255;
  const float alpha = 0.2; // smoothing factor (0=none, 1=instant)
  if (isConnected && (millis() - lastMillis > 50))
  {
    lastMillis = millis();
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY();
    float az = imu.readFloatAccelZ();
    // Calculate pitch and roll in degrees
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    float roll = atan2(ay, az) * 180.0 / PI;
    // Exponential moving average smoothing
    smooth_pitch = alpha * pitch + (1 - alpha) * smooth_pitch;
    smooth_roll = alpha * roll + (1 - alpha) * smooth_roll;
    // Map pitch/roll to MIDI CC (0-127), e.g. -90 to +90 deg -> 0-127
    auto mapCC = [](float v)
    {
      v = constrain(v, -90.0, 90.0);
      return (uint8_t)map(v, -90, 90, 0, 127);
    };
    uint8_t valPitch = mapCC(smooth_pitch);
    uint8_t valRoll = mapCC(smooth_roll);
    uint8_t ccPitch = 21;
    uint8_t ccRoll = 22;
    uint8_t channel = 0;
    uint8_t msgPitch[3] = {static_cast<uint8_t>(0xB0 | channel), ccPitch, valPitch};
    uint8_t msgRoll[3] = {static_cast<uint8_t>(0xB0 | channel), ccRoll, valRoll};
    if (valPitch != lastValPitch)
    {
      midi.send(msgPitch, 3);
      Serial.printf("Sent MIDI CC %d value %d for Pitch: %.2f deg (smoothed)\n", ccPitch, valPitch, smooth_pitch);
      lastValPitch = valPitch;
    }
    if (valRoll != lastValRoll)
    {
      midi.send(msgRoll, 3);
      Serial.printf("Sent MIDI CC %d value %d for Roll: %.2f deg (smoothed)\n", ccRoll, valRoll, smooth_roll);
      lastValRoll = valRoll;
    }
  }
}