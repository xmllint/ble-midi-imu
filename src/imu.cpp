#include "imu.h"
#include <LSM6DS3.h>
#include <Wire.h>

// Create the IMU object (I2C address 0x6A is default for XIAO nRF52840 Sense)
static LSM6DS3 imu(I2C_MODE, 0x6A);

static float smooth_pitch = 0, smooth_roll = 0;
static bool smoothing_initialized = false;

bool imu_init()
{
    if (imu.begin() != 0)
    {
        return false;
    }
    smoothing_initialized = false;
    return true;
}

void imu_read(float &pitch, float &roll)
{
    float ax = imu.readFloatAccelX();
    float ay = imu.readFloatAccelY();
    float az = imu.readFloatAccelZ();
    pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    roll = atan2(ay, az) * 180.0 / PI;
}

void imu_read_smoothed(float &pitch, float &roll, float alpha)
{
    float raw_pitch, raw_roll;
    imu_read(raw_pitch, raw_roll);
    if (!smoothing_initialized)
    {
        smooth_pitch = raw_pitch;
        smooth_roll = raw_roll;
        smoothing_initialized = true;
    }
    smooth_pitch = alpha * raw_pitch + (1 - alpha) * smooth_pitch;
    smooth_roll = alpha * raw_roll + (1 - alpha) * smooth_roll;
    pitch = smooth_pitch;
    roll = smooth_roll;
}
