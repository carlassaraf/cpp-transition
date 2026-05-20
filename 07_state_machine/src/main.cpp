/*
 * Exercise 07 — State Machine with enum class
 *
 * Implement ConnectionFsm and wire it to the buttons and LEDs.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const gpio_dt_spec kBtn1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const gpio_dt_spec kLed2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const gpio_dt_spec kLed3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

/* =========================================================================
 * TODO Part A: Define State and Event with enum class
 * ========================================================================= */

/* Replace these with proper enum class definitions:
 *   States:  Idle, Advertising, Connecting, Connected
 *   Events:  Start, PeerFound, ConnectionDone, Disconnect, Timeout
 */
enum class State  { Idle };  /* TODO: add Advertising, Connecting, Connected */
enum class Event  { Start }; /* TODO: add PeerFound, ConnectionDone, Disconnect, Timeout */

/* =========================================================================
 * TODO Part C: LED update — map each State to a visual LED pattern
 * ========================================================================= */
static void update_leds(State state)
{
    /* Turn all off first */
    gpio_pin_set_dt(&kLed0, 0);
    gpio_pin_set_dt(&kLed1, 0);
    gpio_pin_set_dt(&kLed2, 0);
    gpio_pin_set_dt(&kLed3, 0);

    switch (state) {
    case State::Idle:
        /* LED4 on, others off */
        gpio_pin_set_dt(&kLed3, 1);
        break;
    /* TODO: add cases for Advertising, Connecting, Connected */
    default:
        break;
    }
}

/* =========================================================================
 * TODO Part B: Implement ConnectionFsm
 * ========================================================================= */
class ConnectionFsm {
public:
    ConnectionFsm() : state_(State::Idle) {
        update_leds(state_);
    }

    [[nodiscard]] bool handle(Event event) {
        /* TODO: implement the transition table
         *
         * Idle        + Start          -> Advertising
         * Advertising + PeerFound      -> Connecting
         * Advertising + Timeout        -> Idle
         * Connecting  + ConnectionDone -> Connected
         * Connecting  + Timeout        -> Advertising
         * Connected   + Disconnect     -> Idle
         *
         * All other combinations are invalid -> return false
         */
        (void)event;
        return false;
    }

    State current_state() const { return state_; }

    const char *state_name() const {
        switch (state_) {
        case State::Idle:         return "Idle";
        /* TODO: add remaining cases */
        default:                  return "Unknown";
        }
    }

private:
    State state_;

    void enter_state(State next) {
        printk("FSM: %s -> %s\n", state_name(),
               /* next state name — you'll need a helper or inline strings */
               "?");
        state_ = next;
        update_leds(state_);
    }
};

/* =========================================================================
 * Simple edge-detect helper
 * ========================================================================= */
static bool button_edge(const gpio_dt_spec &spec, bool &last)
{
    bool pressed = (gpio_pin_get_dt(&spec) == 0);  /* active-low */
    bool edge    = pressed && !last;
    last         = pressed;
    return edge;
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    /* Configure all LEDs */
    gpio_pin_configure_dt(&kLed0, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed3, GPIO_OUTPUT_INACTIVE);

    /* Configure buttons */
    gpio_pin_configure_dt(&kBtn0, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&kBtn1, GPIO_INPUT | GPIO_PULL_UP);

    ConnectionFsm fsm;
    printk("Initial state: %s\n", fsm.state_name());

    bool last0 = false, last1 = false;
    /* Sequence Button 1 presses through: Start, PeerFound, ConnectionDone */
    const Event btn0_sequence[] = {Event::Start, Event::Start, Event::Start};
    int btn0_step = 0;

    while (true) {
        if (button_edge(kBtn0, last0)) {
            /* Cycle through events on each press */
            Event ev = btn0_sequence[btn0_step % 3];
            btn0_step++;
            if (!fsm.handle(ev)) {
                printk("Invalid transition ignored\n");
            }
            printk("State: %s\n", fsm.state_name());
        }

        if (button_edge(kBtn1, last1)) {
            /* TODO: send Event::Disconnect */
            printk("State: %s\n", fsm.state_name());
        }

        k_msleep(20);
    }

    return 0;
}
