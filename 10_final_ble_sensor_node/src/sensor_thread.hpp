#pragma once
#include "sensor.hpp"
#include "ring_buffer.hpp"
#include "led.hpp"
#include <zephyr/kernel.h>
#include <functional>

namespace sensor_node {

class SensorThread {
public:
    using SampleCallback = std::function<void(float)>;

    SensorThread(Sensor &sensor, Led &error_led, Led &heartbeat_led);

    void set_sample_callback(SampleCallback cb) { on_sample_ = cb; }
    void start();

    float latest_sample() const;

private:
    static void entry(void *arg, void *, void *);
    void run();

    Sensor &sensor_;
    Led &error_led_;
    Led &heartbeat_led_;
    SampleCallback on_sample_;
    float latest_{0.0f};

    RingBuffer<float, 16> history_;

    K_THREAD_STACK_MEMBER(stack_, 2048);
    k_thread thread_{};
};

} // namespace sensor_node
