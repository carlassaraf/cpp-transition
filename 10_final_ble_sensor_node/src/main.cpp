/*
 * Exercise 10 — Final Project: BLE Sensor Node
 *
 * Integration of all concepts from Exercises 01-09.
 * Your task: implement the TODOs in each component file and
 * wire them together here.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "led.hpp"
#include "sensor.hpp"
#include "ble_manager.hpp"
#include "sensor_thread.hpp"

namespace sensor_node {

/* Device tree specs */
static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const gpio_dt_spec kLed2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const gpio_dt_spec kLed3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);
static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),  gpios);

} // namespace sensor_node

int main(void)
{
    using namespace sensor_node;

    /* --- Hardware objects --- */
    Led led_adv{kLed0};       /* advertising indicator */
    Led led_conn{kLed1};      /* connected indicator   */
    Led led_err{kLed2};       /* sensor error          */
    Led led_beat{kLed3};      /* heartbeat             */

    gpio_pin_configure_dt(&kBtn0, GPIO_INPUT | GPIO_PULL_UP);

    /* --- Sensor and thread --- */
    SimTempSensor sensor;
    SensorThread sensor_thread{sensor, led_err, led_beat};

    /* --- BLE manager --- */
    BleManager &ble = BleManager::instance();
    ble.init();

    /* Wire BLE callbacks using lambdas */
    ble.set_on_connect([&]() {
        led_adv.off();
        led_conn.on();
        printk("Connected — LED2 on\n");
    });

    ble.set_on_disconnect([&]() {
        led_conn.off();
        printk("Disconnected — restarting advertising\n");
        ble.start_advertising();
    });

    /* Forward sensor readings to BLE notify */
    sensor_thread.set_sample_callback([&](float temp) {
        if (ble.state() == BleState::Connected) {
            ble.notify_temperature(temp);
        }
    });

    /* Start everything */
    ble.start_advertising();
    sensor_thread.start();

    printk("=== BLE Sensor Node running ===\n");

    /* Main loop: advertising blink + button handling */
    bool last_btn = false;
    while (true) {
        /* Blink advertising LED when not connected */
        if (ble.state() == BleState::Advertising) {
            led_adv.toggle();
        }

        /* Button 1: force disconnect or restart advertising */
        bool pressed = (gpio_pin_get_dt(&kBtn0) == 0);
        if (pressed && !last_btn) {
            if (ble.state() == BleState::Connected) {
                printk("Button: force disconnect\n");
                ble.disconnect();
            } else if (ble.state() == BleState::Idle) {
                printk("Button: restart advertising\n");
                ble.start_advertising();
            }
        }
        last_btn = pressed;

        k_msleep(500);
    }

    return 0;
}
