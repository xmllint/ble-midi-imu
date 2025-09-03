#pragma once
#include <Arduino.h>

// IMU angle mapping range
#define IMU_ANGLE_MIN -90.0f
#define IMU_ANGLE_MAX 90.0f

// Initialize the IMU. Returns true on success, false on failure.
bool imu_init();

// Read raw pitch and roll (degrees) from IMU.
void imu_read(float &pitch, float &roll);

// Read smoothed pitch and roll (degrees) from IMU, with smoothing factor alpha (0-1).
void imu_read_smoothed(float &pitch, float &roll, float alpha);
