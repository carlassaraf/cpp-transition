/*
 * Exercise 04 — Inheritance and Polymorphism: Sensor Abstraction
 *
 * Implement the two concrete sensor classes and make main() compile.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* =========================================================================
 * Abstract base class — do NOT modify
 * ========================================================================= */
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual float read() = 0;
    virtual const char *name() const = 0;
    bool is_above_threshold() {
        return read() > threshold_;
    }
    void set_threshold(float thres) {
        threshold_ = thres;
    }
protected:
    float threshold_;
};

/* =========================================================================
 * TODO: Implement SimulatedTemperatureSensor
 *
 * - Starts at 20.0f, increments 0.5f per read(), wraps at 30.0f
 * - name() returns "TempSensor"
 * ========================================================================= */
class SimulatedTemperatureSensor : public Sensor {
public:
    SimulatedTemperatureSensor() : temperature_(20.0f) {}

    // Default destructor is generated

    float read() override {
        temperature_ += 0.5f;
        if (temperature_ > 30.0f) {
            temperature_ = 20.0f;
        }
        return temperature_;
    }

    const char *name() const override {
        return "TempSensor";
    }

private:
    float temperature_;
};

/* =========================================================================
 * TODO: Implement SimulatedButtonSensor
 *
 * - Constructor takes const gpio_dt_spec & and configures pin as input
 * - read() returns 1.0f when pressed, 0.0f when released
 * - Buttons on nRF5340 DK are active-low
 * - name() returns "ButtonSensor"
 * ========================================================================= */
class SimulatedButtonSensor : public Sensor {
public:
    explicit SimulatedButtonSensor(const gpio_dt_spec &spec) : spec_(spec) {
        if(!gpio_is_ready_dt(&spec_)) {
            printk("Error: GPIO device not ready\n");
            return;
        }
        gpio_pin_configure_dt(&spec_, GPIO_INPUT);
    }

    // Default constructor is generated

    float read() override {
        // Active-low
        return (gpio_pin_get_dt(&spec_) == 1) ? 1.0f : 0.0f;
    }

    const char *name() const override {
        return "ButtonSensor";
    }

private:
    const gpio_dt_spec &spec_;
};

/* =========================================================================
 * Application — runs a polling loop over all sensors
 * ========================================================================= */
int main(void)
{
    gpio_pin_configure_dt(&kLed0, GPIO_OUTPUT_INACTIVE);

    SimulatedTemperatureSensor temp;
    SimulatedButtonSensor button{kBtn0};

    temp.set_threshold(25.0);
    button.set_threshold(0.0);

    /* Polymorphic array — works for any Sensor subclass */
    Sensor *sensors[] = {&temp, &button};

    while (true) {
        for (auto *s : sensors) {
            float val = s->read();
            printk("%s: %d.%d\n",
                   s->name(),
                   static_cast<int>(val),
                   static_cast<int>((val - static_cast<int>(val)) * 10));
            printk("%s\n", s->is_above_threshold()? "Over threshold" : "Not over threshold");
        }

        gpio_pin_toggle_dt(&kLed0);
        k_msleep(1000);
    }

    return 0;
}
