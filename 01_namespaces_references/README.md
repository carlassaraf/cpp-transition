# Exercise 01 — Namespaces, `const`/`constexpr`, and References

## Goal

Take a classic C-style LED blink and rewrite it using foundational C++ features.
By the end you will understand why these changes make the code safer and more readable
before you even touch classes.

---

## Concepts Covered

| C pattern | C++ replacement | Why it matters |
|---|---|---|
| `#define BLINK_MS 500` | `constexpr uint32_t kBlinkMs = 500;` | Type-safe, debugger-visible, no macro pitfalls |
| Global variables loose in a `.c` file | Variables inside a `namespace` | Avoids name collisions across translation units |
| `void toggle(struct gpio_dt_spec *led)` | `void toggle(const gpio_dt_spec &led)` | References cannot be null; `const &` documents intent |
| `int get_state(struct gpio_dt_spec *led)` | `bool is_active(const gpio_dt_spec &led)` | Proper return type instead of int-as-bool |

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| `constexpr` vs `#define`, compile-time constants | **Chapter 1** — *Introduction / C vs C++* | [`Chapter01/compile_time_const.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/compile_time_const.cpp) vs [`compile_time_const.c`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/compile_time_const.c) |
| References (`&`), rvalue refs, function overloading | **Chapter 6** — *Advanced C++ Language Features* | [`Chapter06/lvalue_refs.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter06/lvalue_refs.cpp), [`Chapter06/function_overloading.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter06/function_overloading.cpp) |
| Type safety and avoiding `void *` | **Chapter 9** — *Type Safety* | [`Chapter09/type_safety/app/src/main_strong_types.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter09/type_safety/app/src) |

> **Reading tip:** Read Chapter 1 first (compare `compile_time_const.c` and `.cpp` side by side on
> [Compiler Explorer](https://godbolt.org)), then skim the Chapter 6 reference examples
> before doing Parts C and D of this exercise.

---

## Hardware

- nRF5340 DK
- LED1 (alias `led0`) — blinks at a configurable interval
- No extra wiring required

---

## The Exercise

The starter code in `src/main.cpp` contains a working C-style LED blink (using the
Zephyr GPIO API) and a set of `TODO` comments marking what you need to change.

### Part A — `constexpr` instead of `#define`

Replace every `#define` constant with a typed `constexpr` value.

### Part B — Namespace

Wrap everything that is not `main()` inside a namespace called `blink`.
Keep `main()` in the global namespace.

### Part C — References

Change every function that takes a `gpio_dt_spec *` pointer to take a
`const gpio_dt_spec &` reference instead. Notice how the call sites
simplify (no `&` needed at the call site because the object is not a raw pointer).

### Part D — `const` correctness

Mark every function that does not modify its argument as taking a `const` reference.
Mark local variables that never change as `const`.

---

## Build & Flash

```bash
# From the exercise directory
west build -b nrf5340dk/nrf5340/cpuapp
west flash --runner jlink
```

> **Note:** The default `nrfutil` runner requires Nordic's nRF Util to be installed.
> If you get a `nrfutil not found` error, use `--runner jlink` instead (requires
> J-Link tools, which ship with the nRF5340-DK board package).

Or use the **nRF Connect for VS Code** extension:
1. Open this folder as the application root.
2. Select board `nrf5340dk/nrf5340/cpuapp`.
3. Click **Build** then **Flash**.

---

## Bonus Challenges

1. Add a second LED (`led1`) that blinks at twice the speed. Put its interval constant
   in the same `blink` namespace.
2. Create a `blink::Config` struct holding `interval_ms` and `led` spec; pass it to
   a `blink::run(const Config &cfg)` function. Observe how a struct passed by reference
   is cleaner than a growing parameter list.
3. Try passing the `gpio_dt_spec` by value and by pointer: measure (or reason about)
   what the compiler generates differently.

---

## Additional Resources

- [Zephyr GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html)
- [cppreference — Namespaces](https://en.cppreference.com/w/cpp/language/namespace)
- [cppreference — constexpr](https://en.cppreference.com/w/cpp/language/constexpr)
- [cppreference — References](https://en.cppreference.com/w/cpp/language/reference)
- nRF5340 DK User Guide — GPIO section (see `materials/` folder)
