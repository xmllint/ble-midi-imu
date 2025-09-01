// BLE MIDI example for nRF52840
#include <bluefruit.h>

// Create the BLE MIDI service
BLEMidi midi;

void setup()
{
  Bluefruit.begin();
  Bluefruit.setName("XIAO-BLE");

  // Start the BLE MIDI service
  midi.begin();

  // Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addService(midi);
  Bluefruit.Advertising.start(0); // Advertise forever
}

void loop()
{
  // Send a MIDI CC message (e.g., Mod Wheel, CC1, value 64) every second
  static uint32_t lastMillis = 0;
  static bool wasConnected = false;
  bool isConnected = Bluefruit.connected();
  if (isConnected && !wasConnected) {
    Serial.println("[BLE] Connected!");
  }
  wasConnected = isConnected;

  if (isConnected && (millis() - lastMillis > 1000))
  {
    lastMillis = millis();
    uint8_t channel = 0; // MIDI channel 1
    uint8_t cc = 1;      // Modulation Wheel (CC1)
    uint8_t value = 64;  // Example value
    uint8_t msg[3] = {static_cast<uint8_t>(0xB0 | channel), cc, value};
    midi.send(msg, 3);
  }
}