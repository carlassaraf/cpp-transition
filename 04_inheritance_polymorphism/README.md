# Exercise 04 — Inheritance and Polymorphism: Sensor Abstraction

## Goal

Design a sensor abstraction hierarchy using inheritance and virtual functions.
You will write code that works on *any* sensor without knowing the concrete type
at compile time — the foundation of a clean Hardware Abstraction Layer (HAL).

---

## Concepts Covered

- **Base class** with pure virtual function (`= 0`): abstract interface
- **Derived classes**: concrete implementations
- **`virtual` destructor**: always required in a polymorphic base class
- **`override` keyword**: lets the compiler catch typos in your override
- **Polymorphic dispatch**: calling the right `read()` through a base pointer
- **vtable overhead**: what it costs in ROM/RAM, and when to avoid it

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| Class basics, first look at OOP | **Chapter 5** — *OOP Fundamentals* | [`Chapter05/class_basics.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter05/class_basics.cpp) |
| Runtime polymorphism (virtual dispatch) via the Observer pattern | **Chapter 15** — *Observer Pattern* | [`Chapter15/observer/app/src/main_observer_rt.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter15/observer/app/src) — the *runtime* observer is built on virtual functions, exactly as your `Sensor` hierarchy |
| Compile-time polymorphism (CRTP) — zero-overhead alternative | **Chapter 8** — *Templates* | [`Chapter08/tmpt_crtp.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter08/tmpt_crtp.cpp) |
| C++ HAL using abstractions | **Chapter 12** — *C++ Hardware Abstraction Layer* | [`Chapter12/cpp_hal/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter12/cpp_hal) |

> **Reading tip:** Read `Chapter05/class_basics.cpp` for foundation, then
> `Chapter15/observer/app/src/main_observer_rt.cpp` for runtime polymorphism — this is
> essentially your `Sensor *` loop already implemented.
> After completing the exercise, read `main_observer_ct.cpp` in the same folder to see
> the compile-time equivalent (templates vs. virtual).

---

## Hardware

- nRF5340 DK
- No extra peripherals needed — sensors are simulated in software
- UART output (J-Link virtual COM port) to display readings
- LED (`led0`) blinks once per read cycle so you know it's alive

---

## The Exercise

### Class Hierarchy

```
Sensor (abstract)
 ├── SimulatedTemperatureSensor
 └── SimulatedButtonSensor   (uses gpio_dt_spec internally)
```

### Abstract base class

```cpp
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual float read() = 0;       // pure virtual
    virtual const char *name() const = 0;
};
```

### `SimulatedTemperatureSensor`

- Simulates temperature with a sawtooth: starts at 20.0°C, increments by 0.5°C
  per `read()`, wraps back at 30.0°C.
- `name()` returns `"TempSensor"`.

### `SimulatedButtonSensor`

- Takes a `gpio_dt_spec` in its constructor.
- `read()` returns `1.0f` if the button is pressed, `0.0f` otherwise.
- `name()` returns `"ButtonSensor"`.

### Application

Create an array of `Sensor *` pointers (or `std::array`), populate it with
one of each type, then loop over it calling `sensor->read()` and printing
the results with `printk()`.

```cpp
Sensor *sensors[] = { &temp, &button };
for (auto *s : sensors) {
    printk("%s: %.1f\n", s->name(), static_cast<double>(s->read()));
}
```

This is polymorphism: the same loop works regardless of how many sensor types you add.

---

## Build & Flash

```bash
west build -b nrf5340dk/nrf5340/cpuapp
west flash
```

Open a serial terminal (115200 baud) to see the sensor readings.

---

## Things to Observe

- Remove `virtual` from the base class destructor and add a `printk` to both
  destructors. Create and destroy a `Sensor *` pointing to a derived object.
  Observe which destructor gets called — this is the undefined-behaviour bug
  that `virtual ~Sensor()` prevents.
- Try adding a third sensor type (e.g., `SimulatedHumiditySensor`) without
  touching `main()`. This is the Open/Closed principle in action.
- Check the `.map` file: each class with virtual functions adds one vtable entry
  (~4 bytes on Cortex-M). Weigh this against the design benefit.

---

## Bonus Challenges

1. Add a `threshold` member to the base class and a `bool is_above_threshold() const`
   non-virtual method. Demonstrate that base-class methods are shared by all derived types.
2. Implement a `SensorLogger` class that holds a `Sensor &` and logs readings to
   a ring buffer (preview of Exercise 05).
3. Make `SimulatedButtonSensor` non-copyable. Why would copying a GPIO wrapper be
   problematic?

---

## Additional Resources

- [cppreference — Virtual functions](https://en.cppreference.com/w/cpp/language/virtual)
- [cppreference — Abstract classes](https://en.cppreference.com/w/cpp/language/abstract_class)
- [CppCoreGuidelines — C.35: A base class destructor should be either public and virtual, or protected and non-virtual](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c35-a-base-class-destructor-should-be-either-public-and-virtual-or-protected-and-non-virtual)
