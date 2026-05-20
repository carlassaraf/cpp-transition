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

/* -------------------------------------------------------------------------
 * TODO Part A: Replace these #defines with constexpr variables.
 * Hint: constexpr uint32_t kBlinkMs = 500;
 * ------------------------------------------------------------------------- */
#define BLINK_INTERVAL_MS   500
#define INIT_DELAY_MS       100

/* -------------------------------------------------------------------------
 * TODO Part B: Move everything below (except main) into namespace blink { }
 * ------------------------------------------------------------------------- */

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* -------------------------------------------------------------------------
 * TODO Part C & D: Change pointer parameters to const references.
 * Before: void led_init(const struct gpio_dt_spec *spec)
 * After:  void led_init(const gpio_dt_spec &spec)
 * ------------------------------------------------------------------------- */
static int led_init(const struct gpio_dt_spec *spec)
{
    if (!gpio_is_ready_dt(spec)) {
        return -ENODEV;
    }
    return gpio_pin_configure_dt(spec, GPIO_OUTPUT_INACTIVE);
}

static void led_toggle(const struct gpio_dt_spec *spec)
{
    gpio_pin_toggle_dt(spec);
}

static bool led_is_active(const struct gpio_dt_spec *spec)
{
    return gpio_pin_get_dt(spec) > 0;
}

int main(void)
{
    k_msleep(INIT_DELAY_MS);

    if (led_init(&led) != 0) {
        return -1;
    }

    while (true) {
        led_toggle(&led);
        k_msleep(BLINK_INTERVAL_MS);
    }

    return 0;
}
