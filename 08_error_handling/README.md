# Exercise 08 — Error Handling Without Exceptions

## Goal

Learn to propagate errors clearly and safely without `throw`/`catch`, which are
typically disabled in embedded firmware (`-fno-exceptions`).
You will implement a `Result<T, E>` type and use `std::optional<T>` for cases
where absence-of-value is the only failure mode.

---

## Concepts Covered

- **Why exceptions are disabled in embedded**: code size, determinism, ISR restrictions
- **`std::optional<T>`**: express "value or nothing" without a sentinel (-1, nullptr)
- **Custom `Result<T, E>`**: express "value or typed error" — similar to Rust's `Result`
- **`[[nodiscard]]`**: compiler warning when the caller throws away an error
- **Error propagation**: chaining `Result` checks without deeply nested `if`s

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| `std::optional` for absent values | **Chapter 7** — *Error Handling* | [`Chapter07/error_handling/app/src/main_optional.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter07/error_handling/app/src) |
| `std::expected` (C++23) / custom Result type | **Chapter 7** | [`Chapter07/error_handling/app/src/main_expected.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter07/error_handling/app/src) |
| `assert`-based error handling | **Chapter 7** | [`Chapter07/error_handling/app/src/main_assert.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter07/error_handling/app/src) |
| Exception handling (and why not to use it in embedded) | **Chapter 7** + **Chapter 1** | [`Chapter07/error_handling/app/src/main_exceptions.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter07/error_handling/app/src), [`Chapter01/exceptions.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/exceptions.cpp) |

> **Reading tip:** Read all four `Chapter07` source files in the order: `main_assert.cpp` →
> `main_exceptions.cpp` → `main_optional.cpp` → `main_expected.cpp`.
> This progression mirrors the exercise: from C-style asserts through exceptions
> (why they're problematic) to the idioms you will actually use.

---

## Hardware

- nRF5340 DK
- No extra peripherals — this is a software-focused exercise
- UART output to display results

---

## Background: The C Pattern and Its Problems

```c
/* C error handling — easy to ignore */
int temperature = read_sensor();   /* -1 means error... or does it mean -1°C? */
if (temperature < 0) { /* ... */ }

float voltage = read_adc();        /* 0.0f means error... or 0 volts? */
```

Sentinel values mix valid data with error codes. The type system cannot help.

---

## The Exercise

### Part A — `std::optional<T>` for simple cases

Implement a `read_temperature()` function that simulates an I2C sensor.
It randomly "fails" 20% of the time and returns `std::nullopt`.

```cpp
std::optional<float> read_temperature();

auto temp = read_temperature();
if (temp.has_value()) {
    printk("Temp: %d.%d C\n", ...);
} else {
    printk("Sensor read failed\n");
}
```

### Part B — Custom `Result<T, E>`

For operations with multiple failure modes, `optional` is not enough.
Implement:

```cpp
enum class SensorError { NotReady, CrcFail, Timeout, OutOfRange };

template <typename T, typename E>
class Result {
public:
    static Result ok(T value);
    static Result err(E error);

    bool is_ok() const;
    bool is_err() const;
    T value() const;      // asserts is_ok() first
    E error() const;      // asserts is_err() first

private:
    /* TODO: store value or error without dynamic allocation */
};
```

Implement `read_sensor_full()` returning `Result<float, SensorError>`.
Simulate different failures based on a counter.

### Part C — `[[nodiscard]]` enforcement

Mark `read_sensor_full()` as `[[nodiscard]]`. Try calling it without using
the return value — the compiler should warn.

### Part D — Error chain

Write a `log_reading()` function that:
1. Calls `read_sensor_full()`.
2. If `ok`, formats and prints the value.
3. If `err`, prints the error name using a `sensor_error_name()` helper.

---

## Build & Flash

```bash
west build -b nrf5340dk/nrf5340/cpuapp
west flash
```

---

## Things to Observe

- `std::optional` uses 1 extra byte (or word, alignment-depending) for the
  validity flag. Check `sizeof(std::optional<float>)` vs `sizeof(float)`.
- Your `Result<float, SensorError>` stores either a float or an enum — this is
  a discriminated union. Compare it to `std::variant<T, E>` (C++17).
- In production embedded code, many teams use `zephyr_error_t` (int) for the
  Zephyr API layer and `Result<T, AppError>` for application logic. The boundary
  is at the driver wrapper.

---

## Bonus Challenges

1. Implement `Result` using `std::variant<T, E>` instead of a manual union.
2. Add a `map()` method: `result.map([](float f) { return f * 1.8f + 32.0f; })`
   transforms the value only if ok.
3. Add `and_then()` (monadic bind): chain two fallible operations without nesting:
   ```cpp
   read_sensor_full()
       .and_then(convert_to_celsius)
       .and_then(validate_range);
   ```

---

## Additional Resources

- [cppreference — std::optional](https://en.cppreference.com/w/cpp/utility/optional)
- [cppreference — std::variant](https://en.cppreference.com/w/cpp/utility/variant)
- [cppreference — nodiscard attribute](https://en.cppreference.com/w/cpp/language/attributes/nodiscard)
- [CppCoreGuidelines — E.6: Use RAII to prevent leaks](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
