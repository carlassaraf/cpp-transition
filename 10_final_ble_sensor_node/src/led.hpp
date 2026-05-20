#pragma once
#include <zephyr/drivers/gpio.h>

namespace sensor_node {

class Led {
public:
    explicit Led(const gpio_dt_spec &spec);

    Led(const Led &) = delete;
    Led &operator=(const Led &) = delete;

    void on() const;
    void off() const;
    void toggle() const;
    bool is_on() const;

    void blink(int times, uint32_t on_ms, uint32_t off_ms) const;

private:
    const gpio_dt_spec &spec_;
};

} // namespace sensor_node
