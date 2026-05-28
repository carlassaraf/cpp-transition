/*
 * Exercise 06 — Lambdas and Callable Objects
 *
 * Implement ButtonDispatcher and register callbacks in three styles.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <functional>

static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const gpio_dt_spec kBtn1 = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const gpio_dt_spec kBtn2 = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/* =========================================================================
 * Minimal Led class (from Exercise 02) — already complete for you
 * ========================================================================= */
class Led {
public:
    explicit Led(const gpio_dt_spec &spec) : spec_(spec) {
        gpio_pin_configure_dt(&spec_, GPIO_OUTPUT_INACTIVE);
    }
    void on()     const { gpio_pin_set_dt(&spec_, 1); }
    void off()    const { gpio_pin_set_dt(&spec_, 0); }
    void toggle() const { gpio_pin_toggle_dt(&spec_); }
private:
    const gpio_dt_spec &spec_;
};

/* =========================================================================
 * TODO: Implement ButtonDispatcher
 *
 * - Constructor configures the pin as input with pull-up.
 * - set_callback() stores the callable.
 * - poll() detects a low-going edge (released→pressed, active-low buttons)
 *   and invokes the callback exactly once per press event.
 * ========================================================================= */
class ButtonDispatcher {
public:
    using Callback = std::function<void()>;

    explicit ButtonDispatcher(const gpio_dt_spec &spec) : spec_(spec) {
        if(!gpio_is_ready_dt(&spec_)) {
            return;
        }
        gpio_pin_configure_dt(&spec_, GPIO_INPUT);
    }

    void enable_irq(void) {
        gpio_init_callback(&gpio_cb_, isr_handler, BIT(spec_.pin));
        gpio_add_callback(spec_.port, &gpio_cb_);
        gpio_pin_interrupt_configure_dt(&spec_, GPIO_INT_EDGE_TO_ACTIVE);
        k_work_init(&work_, work_handler);
    }

    void set_callback(Callback cb) {
        callback_ = std::move(cb);
    }

    void poll() {
        // Get current state and compare to trigger on flank
        bool curr = gpio_pin_get_dt(&spec_);
        if(curr && !last_pressed_) {
            // Just pressed
            if(callback_) {
                callback_();
            }
        }
        last_pressed_ = curr;
    }

private:
    const gpio_dt_spec &spec_;
    Callback callback_;
    bool last_pressed_{false};
    struct gpio_callback gpio_cb_;
    struct k_work        work_;

    static void isr_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins) {
        ButtonDispatcher *self = CONTAINER_OF(cb, ButtonDispatcher, gpio_cb_);
        k_work_submit(&self->work_);
    }

    static void work_handler(struct k_work *work) {
        ButtonDispatcher *self = CONTAINER_OF(work, ButtonDispatcher, work_);
        if (self->callback_) {
            self->callback_();
        }
    }
};

/* =========================================================================
 * Part A — Plain function callback (Style 1)
 * ========================================================================= */
static Led *g_led0_ptr = nullptr;   /* set in main before registering */

static void on_button0_press()
{
    if (g_led0_ptr) {
        g_led0_ptr->toggle();
    }
    printk("Button 0: function pointer callback\n");
}

/* =========================================================================
 * main — register three callback styles, then poll in a loop
 * ========================================================================= */
int main(void)
{
    Led led0{kLed0};
    Led led1{kLed1};
    g_led0_ptr = &led0;

    ButtonDispatcher disp0{kBtn0};
    ButtonDispatcher disp1{kBtn1};
    ButtonDispatcher disp2{kBtn2};

    /* Style 1: plain function pointer */
    disp0.set_callback(on_button0_press);

    /* Style 2: lambda with capture — TODO: write the lambda body */
    int press_count = 0;
    disp1.set_callback([&press_count, &led1]() {
        led1.toggle();
        printk("Button 1: lambda callback (press_count = %d)\n", ++press_count);
    });
    disp1.enable_irq();

    disp2.set_callback([]() {
        printk("Button 2: lambda callback\n");
    });
    disp2.enable_irq();

    /* Main polling loop */
    while (true) {
        disp0.poll();
        k_msleep(20);   /* 20 ms debounce window */
    }

    return 0;
}
