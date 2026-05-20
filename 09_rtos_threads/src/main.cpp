/*
 * Exercise 09 — C++ RTOS Thread Wrappers
 *
 * Implement the Thread base class and two concrete subclasses.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <atomic>

static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const gpio_dt_spec kLed2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const gpio_dt_spec kBtn0 = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),  gpios);

/* =========================================================================
 * Part C — Shared atomic flag (already complete — observe its usage)
 * ========================================================================= */
static std::atomic<bool> g_flag{false};

/* =========================================================================
 * Part A — Abstract Thread base class
 *
 * Zephyr requires stacks allocated with K_THREAD_STACK_DEFINE at file scope.
 * We use a fixed compile-time size via a template non-type parameter.
 *
 * TODO:
 *  - Implement entry() to cast arg to Thread* and call run()
 *  - Implement start() to call k_thread_create()
 *  - Mark the class non-copyable
 * ========================================================================= */
template <size_t StackSize>
class Thread {
public:
    explicit Thread(int priority) : priority_(priority) {
        k_thread_stack_alloc_init();
    }

    virtual ~Thread() = default;

    /* Deleted to prevent slicing and double-free */
    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;

    void start() {
        /* TODO: call k_thread_create with entry() as the thread function,
         * passing 'this' as the first argument.
         *
         * k_thread_create(&thread_, stack_, K_THREAD_STACK_SIZEOF(stack_),
         *                 entry, this, nullptr, nullptr,
         *                 priority_, 0, K_NO_WAIT);
         */
    }

protected:
    virtual void run() = 0;

private:
    static void entry(void *arg, void *, void *) {
        /* TODO: cast arg back to Thread* and call run() */
    }

    K_THREAD_STACK_MEMBER(stack_, StackSize);
    k_thread thread_{};
    int priority_;

    void k_thread_stack_alloc_init() {
        /* Stack is statically declared via K_THREAD_STACK_MEMBER — no-op */
    }
};

/* =========================================================================
 * Part B — Concrete thread: blinks an LED at a fixed interval
 *
 * TODO: inherit from Thread<1024> and implement run()
 * ========================================================================= */
class LedBlinkThread : public Thread<1024> {
public:
    LedBlinkThread(const gpio_dt_spec &led, uint32_t interval_ms, int priority)
        : Thread<1024>(priority), led_(led), interval_ms_(interval_ms)
    {
        gpio_pin_configure_dt(&led_, GPIO_OUTPUT_INACTIVE);
    }

protected:
    void run() override {
        /* TODO: loop forever, toggle led_, sleep interval_ms_ */
    }

private:
    const gpio_dt_spec &led_;
    uint32_t interval_ms_;
};

/* =========================================================================
 * Part B — Concrete thread: mirrors g_flag onto LED3
 *
 * TODO: inherit from Thread<512> and implement run()
 * ========================================================================= */
class FlagMirrorThread : public Thread<512> {
public:
    explicit FlagMirrorThread(const gpio_dt_spec &led, int priority)
        : Thread<512>(priority), led_(led)
    {
        gpio_pin_configure_dt(&led_, GPIO_OUTPUT_INACTIVE);
    }

protected:
    void run() override {
        /* TODO: loop forever, set led_ to match g_flag.load(), sleep 50 ms */
    }

private:
    const gpio_dt_spec &led_;
};

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    gpio_pin_configure_dt(&kBtn0, GPIO_INPUT | GPIO_PULL_UP);

    LedBlinkThread thread_a{kLed0, 200, 5};
    LedBlinkThread thread_b{kLed1, 700, 5};
    FlagMirrorThread thread_c{kLed2, 6};

    thread_a.start();
    thread_b.start();
    thread_c.start();

    printk("Threads started. Press Button 1 to toggle shared flag.\n");

    bool last_btn = false;
    while (true) {
        bool pressed = (gpio_pin_get_dt(&kBtn0) == 0);
        if (pressed && !last_btn) {
            bool new_val = !g_flag.load();
            g_flag.store(new_val);
            printk("Flag toggled: %s\n", new_val ? "true" : "false");
        }
        last_btn = pressed;
        k_msleep(20);
    }

    return 0;
}
