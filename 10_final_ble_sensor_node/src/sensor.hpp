#pragma once
#include <optional>

namespace sensor_node {

enum class SensorError { NotReady, CrcFail, Timeout, OutOfRange };

const char *sensor_error_name(SensorError e);

template <typename T, typename E>
class Result {
public:
    static Result ok(T value);
    static Result err(E error);
    bool is_ok()  const;
    bool is_err() const;
    T value() const;
    E error() const;
private:
    Result() = default;
    /* TODO: implement storage (from Exercise 08) */
};

/* Abstract base */
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual Result<float, SensorError> read() = 0;
    virtual const char *name() const = 0;
};

/* Simulated temperature sensor */
class SimTempSensor : public Sensor {
public:
    SimTempSensor();
    Result<float, SensorError> read() override;
    const char *name() const override { return "SimTemp"; }
private:
    float temp_{20.0f};
    int call_count_{0};
};

} // namespace sensor_node
