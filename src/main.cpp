
#include <Arduino.h>
#include <Wire.h>
#include <bluefruit.h>
#include "imu.h"

// Configuration
constexpr int IMU_ERROR_DELAY_MS = 1000;
constexpr uint8_t MIDI_CC_PITCH = 21;
constexpr uint8_t MIDI_CC_ROLL = 22;
constexpr float SMOOTHING_ALPHA = 0.2f;
constexpr uint8_t MIDI_CHANNEL = 0; // MIDI channel 0 means channel 1 in MIDI (channels are 1-16, but encoded as 0-15)
constexpr uint32_t MIDI_SEND_INTERVAL_MS = 50;

// Create the BLE MIDI service
BLEMidi midi;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Bluefruit.begin();
  Bluefruit.setName("XIAO-BLE");

  // Start the BLE MIDI service
  midi.begin();

  // BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE: Advertise as LE-only, general discoverable mode (standard for BLE MIDI)
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addService(midi);
  Bluefruit.Advertising.start(0); // Advertise indefinitely until connected

  // Initialize IMU
  if (!imu_init())
  {
    Serial.println("Failed to initialize LSM6DS3 IMU!");
    while (1)
      delay(IMU_ERROR_DELAY_MS);
  }
  Serial.println("LSM6DS3 IMU initialized.");
}

// Map a float angle to MIDI CC value (0-127), e.g. -90 to +90 deg -> 0-127
static uint8_t mapCC(float v)
{
  v = constrain(v, IMU_ANGLE_MIN, IMU_ANGLE_MAX);
  return (uint8_t)map(v, IMU_ANGLE_MIN, IMU_ANGLE_MAX, 0, 127);
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
  static uint8_t lastValPitch = 255, lastValRoll = 255;
  if (isConnected && (millis() - lastMillis > MIDI_SEND_INTERVAL_MS))
  {
    lastMillis = millis();
    float pitch, roll;
    imu_read_smoothed(pitch, roll, SMOOTHING_ALPHA);
    // Map pitch/roll to MIDI CC (0-127)
    uint8_t valPitch = mapCC(pitch);
    uint8_t valRoll = mapCC(roll);
    uint8_t msgPitch[3] = {static_cast<uint8_t>(0xB0 | MIDI_CHANNEL), MIDI_CC_PITCH, valPitch};
    uint8_t msgRoll[3] = {static_cast<uint8_t>(0xB0 | MIDI_CHANNEL), MIDI_CC_ROLL, valRoll};
    if (valPitch != lastValPitch)
    {
      midi.send(msgPitch, 3);
      Serial.printf("Sent MIDI CC %d value %d for Pitch: %.2f deg (smoothed)\n", MIDI_CC_PITCH, valPitch, pitch);
      lastValPitch = valPitch;
    }
    if (valRoll != lastValRoll)
    {
      midi.send(msgRoll, 3);
      Serial.printf("Sent MIDI CC %d value %d for Roll: %.2f deg (smoothed)\n", MIDI_CC_ROLL, valRoll, roll);
      lastValRoll = valRoll;
    }
  }
}