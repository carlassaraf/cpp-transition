#include "sensor_thread.hpp"
#include <zephyr/sys/printk.h>

namespace sensor_node {

SensorThread::SensorThread(Sensor &sensor, Led &error_led, Led &heartbeat_led)
    : sensor_(sensor), error_led_(error_led), heartbeat_led_(heartbeat_led)
{
}

void SensorThread::start()
{
    k_thread_create(&thread_, stack_, K_THREAD_STACK_SIZEOF(stack_),
                    entry, this, nullptr, nullptr,
                    7, 0, K_NO_WAIT);
    k_thread_name_set(&thread_, "sensor");
}

float SensorThread::latest_sample() const
{
    return latest_;
}

void SensorThread::entry(void *arg, void *, void *)
{
    static_cast<SensorThread *>(arg)->run();
}

void SensorThread::run()
{
    while (true) {
        heartbeat_led_.toggle();

        auto result = sensor_.read();
        if (result.is_ok()) {
            latest_ = result.value();
            history_.push(latest_);

            printk("[Sensor] %.1f C\n", static_cast<double>(latest_));

            if (on_sample_) {
                on_sample_(latest_);
            }
        } else {
            printk("[Sensor] Error: %s\n", sensor_error_name(result.error()));
            error_led_.blink(3, 100, 100);
        }

        k_msleep(200);
    }
}

} // namespace sensor_node
