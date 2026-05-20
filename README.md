# C++ for Embedded Systems — nRF5340 / Zephyr RTOS Learning Path

A hands-on exercise set for embedded engineers transitioning from C to modern C++.
Each exercise targets a specific idiom, runs on the **nRF5340 DK** with **Zephyr RTOS**,
and maps directly to chapters of the reference book.

---

## Reference Material

| Resource | Location |
|---|---|
| *C++ in Embedded Systems* (Packt) | [`materials/c-in-embedded-systems-a-practical-transition-from-c-to-modern-c.pdf`](materials/c-in-embedded-systems-a-practical-transition-from-c-to-modern-c.pdf) |
| Book code examples (GitHub) | [github.com/PacktPublishing/Cpp-in-Embedded-Systems](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems) |
| nRF5340 DK User Guide | [`materials/nRF5340_DK_User_Guide_v2.0.2.pdf`](materials/nRF5340_DK_User_Guide_v2.0.2.pdf) |

---

## Book Chapter Map

The reference book has 18 chapters. Here is what each covers and where you will use it:

| Chapter | Topic | Used in |
|---|---|---|
| **Ch 1** | Intro to C++: `constexpr`, ring buffers, RTTI, code bloat | Ex 01, Ex 05 |
| **Ch 2** | Memory management: `vector`, PMR, `new`/`delete`, terminate | Ex 03 (background) |
| **Ch 3** | Testing & debugging: Google Test, UB, uninitialized values | General practice |
| **Ch 4** | Bare-metal C++ project structure | Ex 02 (project template) |
| **Ch 5** | OOP basics: classes, constructors, GSM library example | Ex 02, Ex 04 |
| **Ch 6** | Advanced features: references, rvalue refs, move, algorithms | Ex 01, Ex 02 |
| **Ch 7** | Error handling: assert, exceptions, `optional`, `expected` | Ex 08 |
| **Ch 8** | Templates: functions, specialization, CRTP, `enable_if` | Ex 05 |
| **Ch 9** | Type safety: strong types, `reinterpret_cast`, type punning | Ex 07, Ex 09 |
| **Ch 10** | Lambdas: basics, capture lists, `std::function` command pattern | Ex 06 |
| **Ch 11** | Compile-time computation: `constexpr`, lookup tables | Ex 05 (advanced) |
| **Ch 12** | C++ HAL: register abstraction, type-safe peripherals | Ex 10 |
| **Ch 13** | RAII + C API wrapping: LittleFS, UART wrapper | Ex 03, Ex 09 |
| **Ch 14** | Sequencer / task scheduling pattern | Ex 10 (architecture) |
| **Ch 15** | Observer pattern: compile-time and runtime variants | Ex 04, Ex 10 |
| **Ch 16** | Finite State Machine (FSM) | Ex 07, Ex 10 |
| **Ch 17** | C/C++ interface boundary (CIB) | Ex 09, Ex 10 |
| **Ch 18** | Real-world example: accelerometer + tap detection | Ex 10 (reference) |

---

## Exercises

All exercises are independent Zephyr applications — open any folder in VS Code,
select board `nrf5340dk/nrf5340/cpuapp`, build, and flash.

### Exercise Progression

```
01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09 ──► 10 (Final)
C++     OOP    RAII  Poly  Tmpl  λ     FSM   Err   RTOS   Integration
basics  class  lock  morph buf   cb    enum  hdl   wrap   BLE node
```

---

### [01 — Namespaces, `constexpr`, References](01_namespaces_references/)

**Book:** Ch 1 (constexpr), Ch 6 (references), Ch 9 (type safety)
**Book code:** `Chapter01/compile_time_const.cpp`, `Chapter06/lvalue_refs.cpp`

Take a C-style LED blink and refactor it to C++. No classes yet — just the
fundamental language improvements: `constexpr` over `#define`, namespaces to
organise globals, and `const &` references instead of raw pointers.

---

### [02 — GPIO Classes: `Led` and `Button`](02_gpio_class/)

**Book:** Ch 5 (class basics), Ch 6 (const member functions)
**Book code:** `Chapter05/class_basics.cpp`, `Chapter05/gsm_lib/`

Build your first hardware abstraction class. Implement `Led` and `Button`
with proper constructors and `const` member functions, then drive a
knight-rider LED sweep.

---

### [03 — RAII: `ScopedLock`](03_raii_mutex/)

**Book:** Ch 13 (RAII + C API wrapping), Ch 2 (memory / terminate)
**Book code:** `Chapter13/lfs_raii/`, `Chapter13/uart_c_hal_wrapper/`

Wrap Zephyr's `k_mutex` in a `ScopedLock` class whose destructor calls
`k_mutex_unlock()` automatically. Two threads share a counter safely
without a single explicit unlock call.

---

### [04 — Inheritance & Polymorphism: Sensor Hierarchy](04_inheritance_polymorphism/)

**Book:** Ch 5 (OOP), Ch 15 (Observer — runtime polymorphism), Ch 8 (CRTP — compile-time polymorphism)
**Book code:** `Chapter15/observer/app/src/main_observer_rt.cpp`, `Chapter08/tmpt_crtp.cpp`

Define an abstract `Sensor` base class with a pure-virtual `read()`. Implement
two concrete sensors and drive them through a polymorphic pointer loop — the
foundation of every C++ HAL.

---

### [05 — Templates: Type-Safe Ring Buffer](05_templates_ring_buffer/)

**Book:** Ch 1 (ring buffer C→C++ comparison), Ch 8 (templates)
**Book code:** `Chapter01/ring_buffer.cpp`, `Chapter01/ring_buffer_type_erasure.c`, `Chapter08/template_function.cpp`

Replace a `void *` ring buffer with `RingBuffer<T, N>`. The size `N` is a
compile-time constant — zero heap, full type safety. Includes a self-test
that runs on the device and prints PASS/FAIL over UART.

---

### [06 — Lambdas & `std::function` Callbacks](06_lambdas_callbacks/)

**Book:** Ch 10 (lambdas, `std::function` command pattern)
**Book code:** `Chapter10/lambdas/app/src/main_lambda_basics.cpp`, `Chapter10/lambdas/app/src/main_std_function_command_pattern.cpp`

Implement `ButtonDispatcher` with a `std::function` callback slot. Register
the same dispatcher with three different callback styles: a plain function
pointer, a capturing lambda, and a lambda that closes over an `Led` object.

---

### [07 — State Machine with `enum class`](07_state_machine/)

**Book:** Ch 16 (FSM), Ch 9 (type safety / scoped enums)
**Book code:** `Chapter16/fsm/`, `Chapter09/type_safety/app/src/main_strong_types.cpp`

Build a BLE connection lifecycle FSM using `enum class` for states and events.
Invalid transitions return `false` (`[[nodiscard]]` enforces the check).
Four LEDs give visual feedback for each state.

---

### [08 — Error Handling without Exceptions](08_error_handling/)

**Book:** Ch 7 (all four error-handling strategies)
**Book code:** `Chapter07/error_handling/app/src/` — all four `main_*.cpp` files

Implement `std::optional<T>` usage and a custom `Result<T, E>` type.
A simulated sensor returns typed errors (`CrcFail`, `Timeout`, `OutOfRange`)
instead of magic sentinel values.

---

### [09 — C++ RTOS Thread Wrappers](09_rtos_threads/)

**Book:** Ch 13 (C API wrapping), Ch 17 (C/C++ interface boundary)
**Book code:** `Chapter13/uart_c_hal_wrapper/`, `Chapter17/cib/`

Wrap `k_thread_create` in a `Thread<StackSize>` abstract base class. The
static `entry()` trampoline casts `void *arg` back to `Thread *` to call
`run()`. Shared state between threads uses `std::atomic<bool>`.

---

### [10 — Final: BLE Sensor Node](10_final_ble_sensor_node/)

**Book:** Ch 4, 5, 7, 8, 9, 10, 12, 13, 15, 16, 17, 18
**Book code:** `Chapter12/cpp_hal/`, `Chapter15/observer/`, `Chapter16/fsm/`, `Chapter18/`

A complete BLE peripheral that advertises simulated temperature data.
Multi-file project: `Led`, `SimTempSensor`, `RingBuffer<float,16>`,
`BleManager` (FSM + lambda callbacks), and `SensorThread` (thread wrapper).
You design the architecture — the README provides requirements and a
self-assessment checklist.

---

## Quick Start

```bash
# Prerequisites: NCS toolchain + nRF Connect for VS Code installed

# Build and flash any exercise
cd 01_namespaces_references
west build -b nrf5340dk/nrf5340/cpuapp
west flash

# Open serial terminal (J-Link virtual COM port, 115200 baud)
# macOS / Linux
screen /dev/tty.usbmodem* 115200
# or
minicom -D /dev/ttyACM0 -b 115200
```

Or in VS Code: open the exercise folder → **nRF Connect** panel →
select board `nrf5340dk/nrf5340/cpuapp` → **Build** → **Flash**.

---

## Toolchain

- [nRF Connect SDK (NCS)](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html)
- [nRF Connect for VS Code](https://nrfconnect.github.io/vscode-nrf-connect/)
- Zephyr RTOS (bundled with NCS)
- CMake ≥ 3.20, C++17

---

## Suggested Reading Order per Exercise

The fastest path through the book material for each exercise:

| Before Exercise | Read first |
|---|---|
| 01 | Ch 1 pp. 1–end |
| 02 | Ch 5 (class basics), Ch 4 (project structure) |
| 03 | Ch 13 (`lfs_raii` section) |
| 04 | Ch 5, then Ch 15 (`main_observer_rt.cpp`) |
| 05 | Ch 1 (ring buffer files), Ch 8 (template functions) |
| 06 | Ch 10 (all lambda examples) |
| 07 | Ch 16 (FSM), Ch 9 (strong types) |
| 08 | Ch 7 (all four strategies in order) |
| 09 | Ch 13 (`uart_c_hal_wrapper`), Ch 17 |
| 10 | Ch 12, Ch 15, Ch 16, Ch 18 |
