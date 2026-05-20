/*
 * Exercise 02 — Classes and Encapsulation: Led and Button
 *
 * Fill in the class bodies marked with TODO so that the knight-rider
 * sequence in main() compiles and runs on the nRF5340 DK.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Define kit device-tree specs in a namespace to avoid polluting the global namespace
namespace kit {
    // Define constants for the device-tree specs of the LEDs and button
    static const gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
    static const gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
    static const gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
    static const gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);
    static const gpio_dt_spec btn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),  gpios);
};

/* =========================================================================
 * TODO: Implement the Led class.
 *
 * Rules:
 *  - Store a const reference to the gpio_dt_spec (not a copy).
 *  - The constructor must call gpio_is_ready_dt() and gpio_pin_configure_dt().
 *  - on(), off(), toggle(), is_on() must all be const member functions.
 * ========================================================================= */
class Led {
public:
    explicit Led(const gpio_dt_spec &spec) : spec_(spec) {

        if (!gpio_is_ready_dt(&spec_)) {
            return;
        }
        gpio_pin_configure_dt(&spec_, GPIO_OUTPUT_INACTIVE);
    }

    void on() {
        gpio_pin_set_dt(&spec_, 1);
    }

    void off() const {
        gpio_pin_set_dt(&spec_, 0);
    }

    void toggle() const {
        gpio_pin_toggle_dt(&spec_);
    }

    bool is_on() const {
        return gpio_pin_get_dt(&spec_) == GPIO_OUTPUT_ACTIVE;
    }

private:
    const gpio_dt_spec &spec_;
};

/* =========================================================================
 * TODO: Implement the Button class.
 *
 * Rules:
 *  - Configure the pin as input with pull-up (GPIO_INPUT | GPIO_PULL_UP).
 *  - is_pressed() returns true when the button is held down.
 *  - On the nRF5340 DK the buttons are active-low (pressed = logic 0).
 * ========================================================================= */
class Button {
public:
    explicit Button(const gpio_dt_spec &spec) : spec_(spec) {
        if (!gpio_is_ready_dt(&spec_)) {
            return;
        }
        gpio_pin_configure_dt(&spec_, GPIO_INPUT | GPIO_PULL_UP);
    }

    bool is_pressed() const {
        return gpio_pin_get_dt(&spec_);
    }

private:
    const gpio_dt_spec &spec_;
};

/* =========================================================================
 * Application — do NOT modify below this line until you have the classes
 * working. Then feel free to extend it for the bonus challenges.
 * ========================================================================= */

int main(void)
{
    Led leds[] = {
        Led{kit::led0}, Led{kit::led1}, Led{kit::led2}, Led{kit::led3}
    };
    Button btn{kit::btn0};

    /* Knight-rider sweep pattern: 0,1,2,3,2,1,0,1,... */
    const int pattern[] = {0, 1, 2, 3, 2, 1};
    int step = 0;

    while (true) {
        /* Wait while button is held */
        bool pressed = true;
        do {
            pressed = btn.is_pressed();
            k_msleep(10);
        } while (!pressed);

        /* Turn all off, then light the current step */
        for (auto &led : leds) {
            led.off();
        }
        leds[pattern[step]].on();

        step = (step + 1) % 6;
        k_msleep(150);
    }

    return 0;
}
