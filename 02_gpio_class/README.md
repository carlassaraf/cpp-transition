# Exercise 02 — Classes and Encapsulation: `Led` and `Button`

## Goal

Build `Led` and `Button` C++ classes that wrap the Zephyr GPIO API.
You will learn how encapsulation eliminates repetitive boilerplate, makes invalid
use harder, and gives each hardware peripheral a clear, self-documenting interface.

---

## Concepts Covered

- **Class** definition: `public` interface vs `private` data
- **Constructor**: initialisation instead of a separate `init()` function
- **`const` member functions**: express "this call does not change state"
- **Encapsulation**: callers cannot accidentally pass the wrong GPIO spec to the wrong function
- **`this` pointer**: implicit inside member functions
- **Ownership and invariants**: the object is always in a valid state after construction

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| Class fundamentals, constructors, member functions | **Chapter 5** — *OOP Fundamentals / Class Basics* | [`Chapter05/class_basics.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter05/class_basics.cpp) — also look at the `gsm_lib/` folder for a full class-based library |
| `const` member functions, `const` references | **Chapter 6** — *Advanced C++ Language Features* | [`Chapter06/lvalue_refs.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter06/lvalue_refs.cpp) |
| Bare-metal C++ project structure | **Chapter 4** — *Bare Metal C++* | [`Chapter04/bare/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter04/bare) |

> **Reading tip:** Study `Chapter05/class_basics.cpp` before implementing `Led`.
> Pay attention to how the GSM library (`Chapter05/gsm_lib/`) structures a real hardware driver
> as a class — it's the same pattern you're applying here.

---

## Hardware

- nRF5340 DK
- LED1 (`led0`), LED2 (`led1`), LED3 (`led2`), LED4 (`led3`)
- Button 1 (`sw0`)

---

## The Exercise

The starter code declares `class Led` with all four methods stubbed out and
`class Button` completely empty. Fill them in so that `main()` compiles and
runs the traffic-light sequence described below.

### Required `Led` interface

```cpp
class Led {
public:
    Led(const gpio_dt_spec &spec);  // configure pin as output
    void on() const;
    void off() const;
    void toggle() const;
    bool is_on() const;
private:
    const gpio_dt_spec &spec_;
};
```

### Required `Button` interface

```cpp
class Button {
public:
    Button(const gpio_dt_spec &spec);  // configure pin as input
    bool is_pressed() const;
private:
    const gpio_dt_spec &spec_;
};
```

### Application behaviour

1. On boot, run a "knight rider" sweep: LED1 → LED2 → LED3 → LED4 → LED3 → LED2, repeat.
2. When Button 1 is pressed, freeze on the current LED and hold until released.
3. After release, resume the sweep.

This forces you to use all four `Led` methods and to poll `Button::is_pressed()`.

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

- Compare how the `main()` in this exercise reads vs the raw Zephyr API calls
  you wrote in Exercise 01. Notice that the intent ("turn led on") is now explicit.
- Try calling `led.off()` without `const` on the method — the compiler refuses when
  `led` is `const Led`. This is the compiler enforcing correctness for free.
- Notice that if you forget to call the Zephyr `gpio_pin_configure_dt()` somewhere,
  the LED simply never works. The constructor is the natural place to guarantee it runs.

---

## Bonus Challenges

1. Add a `Led::blink(uint32_t on_ms, uint32_t off_ms)` convenience method.
2. Make the constructor `[[nodiscard]]`-warn if init fails: return a `bool` from a static
   factory function `static std::optional<Led> create(const gpio_dt_spec &spec)` instead
   of using a constructor that silently fails. (Preview of Exercise 08.)
3. Prevent copying a `Led` (delete the copy constructor and copy-assignment operator).
   What happens when you try `Led led2 = led1;`?

---

## Additional Resources

- [Zephyr GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- [cppreference — Classes](https://en.cppreference.com/w/cpp/language/classes)
- [cppreference — Constructors](https://en.cppreference.com/w/cpp/language/constructor)
- nRF5340 DK User Guide — LED and Button pin mapping (see `materials/` folder)
