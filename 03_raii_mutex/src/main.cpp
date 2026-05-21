/*
 * Exercise 03 — RAII: Resource Acquisition Is Initialization
 *
 * Implement ScopedLock and SharedCounter so that the two threads
 * below share a counter safely without any explicit unlock() calls.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

static const gpio_dt_spec kLed0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const gpio_dt_spec kLed1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);

/* =========================================================================
 * TODO Part A: Implement ScopedLock
 *
 * - Constructor must call k_mutex_lock(&mutex_, K_FOREVER)
 * - Destructor must call k_mutex_unlock(&mutex_)
 * - Delete copy constructor and copy-assignment operator
 * ========================================================================= */
class ScopedLock {
public:
    explicit ScopedLock(k_mutex &mutex) : mutex_(mutex) {
        k_mutex_lock(&mutex_, K_FOREVER);
    }

    ~ScopedLock() {
        // k_mutex_unlock(&mutex_);
    }

    /* TODO: delete copy constructor and copy-assignment operator */
    ScopedLock(const ScopedLock &) = delete;
    ScopedLock &operator=(const ScopedLock &) = delete;

private:
    k_mutex &mutex_;
};

/* =========================================================================
 * TODO Part B: Implement SharedCounter
 *
 * Use ScopedLock inside increment() and get() — no explicit unlock() calls.
 * ========================================================================= */
class SharedCounter {
public:
    SharedCounter() : value_(0) {
        k_mutex_init(&mutex_);
    }

    void increment() {
        /* TODO: create a ScopedLock, then increment value_ */
        ScopedLock lock(mutex_);
        value_++;
    }

    int get() const {
        /* TODO: create a ScopedLock, then return value_ */
        ScopedLock lock(mutex_);
        return value_;
    }

private:
    int value_;
    mutable k_mutex mutex_;
};

/* =========================================================================
 * Global shared state
 * ========================================================================= */
static SharedCounter g_counter;

/* =========================================================================
 * Thread A: increments the counter every 200 ms, blinks LED1
 * ========================================================================= */
static void thread_a(void *, void *, void *)
{
    gpio_pin_configure_dt(&kLed0, GPIO_OUTPUT_INACTIVE);

    while (true) {
        g_counter.increment();
        gpio_pin_toggle_dt(&kLed0);
        k_msleep(50);
    }
}

/* =========================================================================
 * Thread B: reads counter every 500 ms, blinks LED2 every 1000 increments
 * ========================================================================= */
static void thread_b(void *, void *, void *)
{
    gpio_pin_configure_dt(&kLed1, GPIO_OUTPUT_INACTIVE);
    int last_milestone = 0;

    while (true) {
        int val = g_counter.get();
        int milestone = val / 1000;
        if (milestone > last_milestone) {
            last_milestone = milestone;
            gpio_pin_toggle_dt(&kLed1);
            printk("Counter milestone: %d\n", val);
        }
        k_msleep(500);
    }
}

#define STACK_SIZE 1024
#define PRIORITY   5

K_THREAD_DEFINE(tid_a, STACK_SIZE, thread_a, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(tid_b, STACK_SIZE, thread_b, NULL, NULL, NULL, PRIORITY + 1, 0, 0);

int main(void)
{
    printk("Exercise 03: RAII mutex demo\n");
    return 0;
}
