/*
 * Exercise 01 — Namespaces, const/constexpr, References
 *
 * This file starts in C style. Your job is to migrate it to idiomatic C++
 * following the README instructions. Each TODO marks one specific change.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

namespace blink {

    static int led_init(const struct gpio_dt_spec &spec)
    {
        if (!gpio_is_ready_dt(&spec)) {
            return -ENODEV;
        }
        return gpio_pin_configure_dt(&spec, GPIO_OUTPUT_INACTIVE);
    }

    static void led_toggle(const struct gpio_dt_spec &spec)
    {
        gpio_pin_toggle_dt(&spec);
    }
};

namespace led0_thread {
    // Thread parameters
    struct k_thread led_thread;
    constexpr uint32_t stack_size = 1024;
    constexpr int kPrio = 1;
    K_THREAD_STACK_DEFINE(led_stack, stack_size);

    // Blink and init time
    constexpr uint32_t kBlinkMs = CONFIG_LED0_BLINKY_MS;
    constexpr uint32_t kInitDelayMs = CONFIG_INIT_TIME_MS;

    // LED specification
    const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
    
    // Thread entry point
    void thread_entry(void *, void *, void *)
    {
        if (blink::led_init(led) != 0) {
            return;
        }
        while (true) {
            blink::led_toggle(led);
            k_msleep(kBlinkMs);
        }
    }
};

namespace led1_thread {
    // Thread parameters
    struct k_thread led_thread;
    constexpr uint32_t stack_size = 1024;
    constexpr int kPrio = 1;
    K_THREAD_STACK_DEFINE(led_stack, stack_size);

    // Blink and init time
    constexpr uint32_t kBlinkMs = CONFIG_LED1_BLINKY_MS;
    constexpr uint32_t kInitDelayMs = CONFIG_INIT_TIME_MS;

    // LED specification
    const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

    // Thread entry point
    void thread_entry(void *, void *, void *)
    {
        if (blink::led_init(led) != 0) {
            return;
        }
        while (true) {
            blink::led_toggle(led);
            k_msleep(kBlinkMs);
        }
    }
};

int main(void)
{
    // Create threads for LED0 and LED1

    k_thread_create(
        &led0_thread::led_thread,
        led0_thread::led_stack,
        K_THREAD_STACK_SIZEOF(led0_thread::led_stack),
        led0_thread::thread_entry,
        nullptr, nullptr, nullptr,
        led0_thread::kPrio,
        0, K_MSEC(led0_thread::kInitDelayMs)
    );

    k_thread_create(
        &led1_thread::led_thread,
        led1_thread::led_stack,
        K_THREAD_STACK_SIZEOF(led1_thread::led_stack),
        led1_thread::thread_entry,
        nullptr, nullptr, nullptr,
        led1_thread::kPrio,
        0, K_MSEC(led1_thread::kInitDelayMs)
    );

    while (true) { k_sleep(K_FOREVER); }
    return 0;
}
