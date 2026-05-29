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
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed2 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const gpio_dt_spec kLed3 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const gpio_dt_spec kLed4 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);

// States and Events

enum class State  { Idle, Advertising, Connecting, Connected }; 
enum class Event  { Start, PeerFound, ConnectionDone, Disconnect, Timeout };

// State Interface Class

class StateInterface {
public:
    virtual State handle_event(Event event) = 0;
    virtual State get_state() = 0;
    virtual const char* get_state_name() = 0;
};

// State implementation classes

class Idle : public StateInterface {
public:
    State handle_event(Event event) override {
        if(event == Event::Start) {
            advertising();
            return State::Advertising;
        }
        return State::Idle;
    }

    State get_state() override {
        return State::Idle;
    }

    const char* get_state_name() override {
        return "Idle";
    }

private:
    void advertising() {
        printk("Starting to advertise...\n");
    }
};

class Advertising : public StateInterface {
public:
    State handle_event(Event event) override {
        if(event == Event::PeerFound) {
            device_found();
            return State::Connecting;
        } else if(event == Event::Timeout) {
            timeout();
            return State::Idle;
        }
        return State::Advertising;
    }

    State get_state() override {
        return State::Advertising;
    }

    const char* get_state_name() override {
        return "Advertising";
    }

private:
    void device_found() {
        printk("Device found. Attempting to connect...\n");
    }

    void timeout() {
        printk("No device found. Connection failed...\n");
    }
};

class Connecting : public StateInterface {
public:
    State handle_event(Event event) override {
        if(event == Event::ConnectionDone) {
            connected();
            return State::Connected;
        } else if(event == Event::Timeout) {
            timeout();
            return State::Advertising;
        }
        return State::Connecting;
    }

    State get_state() override {
        return State::Connecting;
    }

    const char* get_state_name() override {
        return "Connecting";
    }

private:
    void connected() {
        printk("Device connected!\n");
    }

    void timeout() {
        printk("Timeout happened. Going back to advertising\n");
    }
};

class Connected : public StateInterface {
public:
    State handle_event(Event event) override {
        if(event == Event::Disconnect) {
            disconnect();
            return State::Idle;
        }
        return State::Connected;
    }

    State get_state() override {
        return State::Connected;
    }

    const char* get_state_name() override {
        return "Connected";
    }

private:
    void disconnect() {
        printk("Disconnecting...\n");
    }
};

/* =========================================================================
 * TODO Part C: LED update — map each State to a visual LED pattern
 * ========================================================================= */
static void update_leds(State state)
{
    /* Turn all off first */
    gpio_pin_set_dt(&kLed1, 0);
    gpio_pin_set_dt(&kLed2, 0);
    gpio_pin_set_dt(&kLed3, 0);
    gpio_pin_set_dt(&kLed4, 0);

    switch (state) {
    case State::Idle:
        /* LED1 on, others off */
        gpio_pin_set_dt(&kLed1, 1);
        break;
    case State::Advertising:
        gpio_pin_set_dt(&kLed2, 1);
        break;
    case State::Connecting:
        gpio_pin_set_dt(&kLed3, 1);
        break;
    case State::Connected:
        gpio_pin_set_dt(&kLed4, 1);
        break;
    default:
        break;
    }
};

/* =========================================================================
 * TODO Part B: Implement ConnectionFsm
 * ========================================================================= */
class ConnectionFsm {
public:
    ConnectionFsm() : current_state_(State::Idle) {
        states_[static_cast<int>(State::Idle)]        = &idle_;
        states_[static_cast<int>(State::Advertising)] = &advertising_;
        states_[static_cast<int>(State::Connecting)]  = &connecting_;
        states_[static_cast<int>(State::Connected)]   = &connected_;
        update_leds(current_state_);
    }

    void handle(Event event) {
        if (auto* state = get_state(current_state_)) {
            current_state_ = state->handle_event(event);
            update_leds(current_state_);
        }
    }

    State current_state() const { return current_state_; }

    const char *current_state_name() {
        if (auto* state = get_state(current_state_)) {
            return state->get_state_name();
        }
        return "Unknown";
    }

    const char *state_name(State state) {
        auto* s = get_state(state);
        return s ? s->get_state_name() : "Unknown";
    }

private:
    State current_state_;
    Idle        idle_;
    Advertising advertising_;
    Connecting  connecting_;
    Connected   connected_;
    StateInterface* states_[4];

    StateInterface* get_state(State state) {
        return states_[static_cast<int>(state)];
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
    gpio_pin_configure_dt(&kLed1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed3, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&kLed4, GPIO_OUTPUT_INACTIVE);

    /* Configure buttons */
    gpio_pin_configure_dt(&kBtn0, GPIO_INPUT | GPIO_PULL_UP);
    gpio_pin_configure_dt(&kBtn1, GPIO_INPUT | GPIO_PULL_UP);

    ConnectionFsm fsm;
    printk("Initial state: %s\n", fsm.current_state_name());

    bool last0 = false, last1 = false;
    /* Sequence Button 1 presses through: Start, PeerFound, ConnectionDone */
    const Event btn0_sequence[] = {Event::Start, Event::PeerFound, Event::ConnectionDone};
    int btn0_step = 0;

    // Timeout counter
    constexpr uint32_t kSleep_ms = 20;
    constexpr uint32_t kTimeout_ms = 5000;
    uint32_t timeout_counter = 0;

    while (true) {
        if (button_edge(kBtn0, last0)) {
            /* Cycle through events on each press */
            Event ev = btn0_sequence[btn0_step % 3];
            btn0_step++;
            fsm.handle(ev);
            printk("State: %s\n", fsm.current_state_name());
        }

        if (button_edge(kBtn1, last1)) {
            fsm.handle(Event::Disconnect);
            printk("State: %s\n", fsm.current_state_name());
        }

        if(fsm.current_state() == State::Advertising || fsm.current_state() == State::Connecting) {
            timeout_counter++;
            if(timeout_counter >= (kTimeout_ms / kSleep_ms)) {
                fsm.handle(Event::Timeout);
                timeout_counter = 0;
            }
        } else if(timeout_counter > 0) { timeout_counter = 0; }

        k_msleep(kSleep_ms);
    }

    return 0;
}
