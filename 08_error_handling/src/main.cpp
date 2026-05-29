/*
 * Exercise 08 — Error Handling Without Exceptions
 *
 * Implement Result<T, E>, std::optional usage, and [[nodiscard]] enforcement.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <optional>
#include <cassert>

/* =========================================================================
 * Part A — std::optional: simple fallible read
 * ========================================================================= */

/* Simulates an I2C temperature sensor that fails ~20% of the time */
static int g_sim_counter = 0;

std::optional<float> read_temperature()
{
    ++g_sim_counter;
    if (g_sim_counter % 5 == 0) {
        return std::nullopt;   /* simulated failure */
    }
    /* Simulate a sawtooth between 20.0 and 29.5 */
    return 20.0f + (g_sim_counter % 20) * 0.5f;
}

/* =========================================================================
 * Part B — Custom Result<T, E>
 * ========================================================================= */

enum class SensorError {
    NotReady,
    CrcFail,
    Timeout,
    OutOfRange,
};

static const char *sensor_error_name(SensorError e)
{
    switch (e) {
    case SensorError::NotReady:   return "NotReady";
    case SensorError::CrcFail:    return "CrcFail";
    case SensorError::Timeout:    return "Timeout";
    case SensorError::OutOfRange: return "OutOfRange";
    default:                      return "Unknown";
    }
}

/*
 * TODO: Implement Result<T, E>
 *
 * Requirements:
 *  - No dynamic allocation
 *  - ok() and err() are static factory methods
 *  - is_ok() / is_err() check the discriminant
 *  - value() asserts is_ok() before returning
 *  - error() asserts is_err() before returning
 *
 * Hint: store either T or E in a union, with a bool discriminant.
 */
template <typename T, typename E>
class Result {
public:
    static Result ok(T value) {
        Result r;
        r.storage_.result = value;
        r.ok_ = true;
        return r;
    }

    static Result err(E error) {
        Result r;
        r.storage_.error = error;
        r.ok_ = false;
        return r;
    }

    bool is_ok()  const { return ok_; }
    bool is_err() const { return !is_ok(); }

    T value() const {
        assert(is_ok());
        return storage_.result;
    }

    E error() const {
        assert(is_err());
        return storage_.error;
    }

private:
    Result() = default;
    union { T result; E error; } storage_;
    bool ok_;
};

/* =========================================================================
 * Part C — [[nodiscard]] fallible sensor
 *
 * Simulates three different failure modes cycling through g_sim_counter.
 * ========================================================================= */
[[nodiscard]]
static Result<float, SensorError> read_sensor_full()
{
    int tick = ++g_sim_counter;

    if (tick % 7 == 0) return Result<float, SensorError>::err(SensorError::Timeout);
    if (tick % 5 == 0) return Result<float, SensorError>::err(SensorError::CrcFail);
    if (tick % 3 == 0) return Result<float, SensorError>::err(SensorError::NotReady);

    float val = 20.0f + (tick % 20) * 0.5f;
    if (val > 29.0f)   return Result<float, SensorError>::err(SensorError::OutOfRange);

    return Result<float, SensorError>::ok(val);
}

/* =========================================================================
 * Part D — Error chain helper
 * ========================================================================= */
static void log_reading()
{
    auto result = read_sensor_full();
    if (result.is_ok()) {
        float v = result.value();
        printk("Sensor OK: %d.%d C\n",
               static_cast<int>(v),
               static_cast<int>((v - static_cast<int>(v)) * 10));
    } else {
        printk("Sensor ERR: %s\n", sensor_error_name(result.error()));
    }
}

/* =========================================================================
 * main
 * ========================================================================= */
int main(void)
{
    printk("=== Part A: std::optional ===\n");
    for (int i = 0; i < 10; ++i) {
        auto temp = read_temperature();
        if (temp.has_value()) {
            float v = temp.value();
            printk("Temp: %d.%d C\n",
                   static_cast<int>(v),
                   static_cast<int>((v - static_cast<int>(v)) * 10));
        } else {
            printk("Temp: read failed\n");
        }
        k_msleep(100);
    }

    g_sim_counter = 0;
    printk("\n=== Part B/C/D: Result<T,E> ===\n");
    while (true) {
        log_reading();
        k_msleep(500);
    }

    return 0;
}
