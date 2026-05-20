#include "sensor.hpp"

namespace sensor_node {

const char *sensor_error_name(SensorError e)
{
    switch (e) {
    case SensorError::NotReady:   return "NotReady";
    case SensorError::CrcFail:    return "CrcFail";
    case SensorError::Timeout:    return "Timeout";
    case SensorError::OutOfRange: return "OutOfRange";
    default:                      return "Unknown";
    }
}

SimTempSensor::SimTempSensor() : temp_(20.0f), call_count_(0) {}

Result<float, SensorError> SimTempSensor::read()
{
    ++call_count_;

    /* Simulate occasional failures */
    if (call_count_ % 13 == 0) return Result<float, SensorError>::err(SensorError::CrcFail);
    if (call_count_ % 7  == 0) return Result<float, SensorError>::err(SensorError::Timeout);

    temp_ += 0.25f;
    if (temp_ > 30.0f) temp_ = 20.0f;

    return Result<float, SensorError>::ok(temp_);
}

} // namespace sensor_node
