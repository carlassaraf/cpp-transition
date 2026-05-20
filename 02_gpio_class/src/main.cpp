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

/* Device-tree specs for all four LEDs and button 1 */
static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const gpio_dt_spec kLed2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const gpio_dt_spec kLed3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);
static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),  gpios);

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
        /* TODO: check device is ready, configure as output */
    }

    void on() const {
        /* TODO */
    }

    void off() const {
        /* TODO */
    }

    void toggle() const {
        /* TODO */
    }

    bool is_on() const {
        /* TODO: return true when the pin is active */
        return false;
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
        /* TODO: check device is ready, configure as input with pull-up */
    }

    bool is_pressed() const {
        /* TODO: active-low — return true when pin reads 0 */
        return false;
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
        Led{kLed0}, Led{kLed1}, Led{kLed2}, Led{kLed3}
    };
    Button btn{kBtn0};

    /* Knight-rider sweep pattern: 0,1,2,3,2,1,0,1,... */
    const int pattern[] = {0, 1, 2, 3, 2, 1};
    int step = 0;

    while (true) {
        /* Wait while button is held */
        while (btn.is_pressed()) {
            k_msleep(10);
        }

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
