# Exercise 06 — Lambdas and Callable Objects

## Goal

Understand the three generations of callbacks in C/C++: raw function pointers,
lambdas (closures), and `std::function`. Implement a simple button event
dispatcher that lets callers register callbacks in any of these styles.

---

## Concepts Covered

- **C-style function pointer**: baseline; what you already use
- **Lambda expression**: anonymous function, defined inline
- **Capture list**: `[&]`, `[=]`, `[this]`, `[var]` — how lambdas close over scope
- **`std::function<void()>`**: type-erased callable; holds any of the above
- **Template callbacks** (`template <typename F>`): zero-overhead alternative to `std::function`
- **ISR constraint**: you cannot call `std::function` from an ISR; learn the safe pattern

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| Lambda basics, capture lists | **Chapter 10** — *Lambdas* | [`Chapter10/lambdas/app/src/main_lambda_basics.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter10/lambdas/app/src) |
| `std::function` and the Command pattern | **Chapter 10** | [`Chapter10/lambdas/app/src/main_std_function_command_pattern.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter10/lambdas/app/src) |
| Function overloading baseline | **Chapter 6** — *Advanced C++ Language Features* | [`Chapter06/function_overloading.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter06/function_overloading.cpp) |

> **Reading tip:** `main_lambda_basics.cpp` covers capture lists step by step — read it
> before implementing `ButtonDispatcher::set_callback()`.
> `main_std_function_command_pattern.cpp` is the direct analogue of the dispatcher pattern:
> the "command" stored in the dispatcher IS the `std::function` you are implementing.

---

## Hardware

- nRF5340 DK
- Button 1 (`sw0`), Button 2 (`sw1`)
- LED1 (`led0`), LED2 (`led1`)
- Zephyr GPIO interrupt for button detection

---

## Background: The C Callback Pattern

```c
typedef void (*button_cb_t)(void *user_data);

void button_register_callback(button_cb_t cb, void *user_data);
```

This works but forces the caller to cast `user_data` back to the right type every time.
The lambda approach inlines that context capture safely.

---

## The Exercise

### Part A — `ButtonDispatcher` class

```cpp
class ButtonDispatcher {
public:
    using Callback = std::function<void()>;

    explicit ButtonDispatcher(const gpio_dt_spec &spec);
    void set_callback(Callback cb);
    void poll();   // call from main loop; fires callback on press edge
private:
    const gpio_dt_spec &spec_;
    Callback callback_;
    bool last_state_{false};
};
```

`poll()` detects a rising press edge (released → pressed) and invokes `callback_`.

### Part B — Register three different callback styles

```cpp
/* Style 1: plain function pointer */
void on_button1_press() { /* toggle led0 */ }
dispatcher1.set_callback(on_button1_press);

/* Style 2: lambda with capture */
int press_count = 0;
dispatcher2.set_callback([&press_count]() {
    ++press_count;
    printk("Button 2 pressed %d times\n", press_count);
});

/* Style 3: lambda capturing an Led object */
Led led{kLed0};
dispatcher1.set_callback([&led]() { led.toggle(); });
```

### Part C — Template callback (bonus)

Rewrite `ButtonDispatcher` as a class template:

```cpp
template <typename F>
class ButtonDispatcherT {
public:
    explicit ButtonDispatcherT(const gpio_dt_spec &spec, F callback);
    void poll();
private:
    const gpio_dt_spec &spec_;
    F callback_;
    bool last_state_{false};
};
```

Compare binary size between the `std::function` and template versions using
the `.map` file or `west build --target rom_report`.

---

## Build & Flash

```bash
west build -b nrf5340dk/nrf5340/cpuapp
west flash --runner jlink
```
> **Note:** The default `nrfutil` runner requires Nordic's nRF Util to be installed.
> If you get a `nrfutil not found` error, use `--runner jlink` instead (requires
> J-Link tools, which ship with the nRF5340-DK board package).

---

## Things to Observe

- `std::function` stores the callable on the heap or uses small-buffer optimisation.
  Check whether your target has `CONFIG_HEAP_MEM_POOL_SIZE` set large enough.
- A raw function pointer is just an address — it cannot capture. A lambda *can*
  capture, but only stateless lambdas (no captures) convert to a function pointer.
- ISR callbacks: Zephyr GPIO interrupt callbacks must be plain C functions.
  The safe pattern is to post to a `k_msgq` from the ISR and process in a thread.

---

## Bonus Challenges

1. Connect a real GPIO interrupt (`gpio_add_callback`) and debounce it by only
   firing the `std::function` from a work-queue item, not the ISR itself.
2. Implement a multi-callback dispatcher: store up to 4 `std::function` callbacks
   and fire all of them on each press.
3. Measure and compare ROM usage for `std::function` vs the template version.

---

## Additional Resources

- [cppreference — Lambda expressions](https://en.cppreference.com/w/cpp/language/lambda)
- [cppreference — std::function](https://en.cppreference.com/w/cpp/utility/functional/function)
- [Zephyr GPIO Interrupt API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- [CppCoreGuidelines — F.50: Use lambdas when a function won't do](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#f50-use-a-lambda-when-a-function-wont-do)
