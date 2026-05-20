#include "led.hpp"
#include <zephyr/kernel.h>

namespace sensor_node {

Led::Led(const gpio_dt_spec &spec) : spec_(spec)
{
    /* TODO: configure GPIO as output */
}

void Led::on() const
{
    /* TODO */
}

void Led::off() const
{
    /* TODO */
}

void Led::toggle() const
{
    /* TODO */
}

bool Led::is_on() const
{
    /* TODO */
    return false;
}

void Led::blink(int times, uint32_t on_ms, uint32_t off_ms) const
{
    for (int i = 0; i < times; ++i) {
        on();
        k_msleep(on_ms);
        off();
        k_msleep(off_ms);
    }
}

} // namespace sensor_node
