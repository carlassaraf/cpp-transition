# Exercise 10 — Final Project: BLE Sensor Node

## Goal

Build a complete BLE peripheral that advertises simulated temperature data.
This project integrates every concept from Exercises 01–09 into a real, running
firmware application with a clean architecture.

This is not a fill-in-the-blanks exercise. You design the architecture.
The README gives you the requirements and the file structure; you write the code.

---

## Skills Applied

| File | Concepts from exercises | Book chapters |
|---|---|---|
| `src/led.hpp/cpp` | Ex 02 (classes), Ex 03 (RAII) | Ch 5, Ch 13 |
| `src/sensor.hpp/cpp` | Ex 04 (inheritance), Ex 08 (Result type) | Ch 5, Ch 7, Ch 15 |
| `src/ring_buffer.hpp` | Ex 05 (templates) | Ch 1, Ch 8 |
| `src/ble_manager.hpp/cpp` | Ex 06 (lambdas), Ex 07 (state machine) | Ch 10, Ch 16 |
| `src/sensor_thread.hpp/cpp` | Ex 09 (thread wrapper) | Ch 13, Ch 17 |
| `src/main.cpp` | Integration | Ch 4, Ch 12, Ch 18 |

## Book Reference — Full Map

| Book Chapter | What it contributes to this project |
|---|---|
| **Ch 1** — Introduction / C vs C++ | Ring buffer comparison; `constexpr`; bloat awareness |
| **Ch 4** — Bare Metal C++ | Project structure template |
| **Ch 5** — OOP Fundamentals | `Led`, `Sensor` class design |
| **Ch 7** — Error Handling | `Result<T,E>`, `std::optional` for sensor reads |
| **Ch 8** — Templates | `RingBuffer<T,N>` generic implementation |
| **Ch 9** — Type Safety | Strong types for sensor values; avoiding `reinterpret_cast` pitfalls |
| **Ch 10** — Lambdas | BLE connect/disconnect lambda callbacks |
| **Ch 12** — C++ HAL | Hardware abstraction structure for GPIO, BLE |
| **Ch 13** — RAII / C API Wrapping | RAII mutex in `SensorThread`; `SensorThread` itself wraps Zephyr C API |
| **Ch 15** — Observer Pattern | `set_sample_callback` is an Observer; `main_observer_rt.cpp` is the model |
| **Ch 16** — FSM | `BleManager` state machine |
| **Ch 17** — C/C++ Interface Boundary | Static trampoline pattern in `SensorThread::entry()` |
| **Ch 18** — Real-World Example | Accelerometer + tap detection: reference for sensor algorithm structure |

> **Suggested reading before starting:** Ch 12 (`cpp_hal`), Ch 15 (`observer`),
> Ch 16 (`fsm`), Ch 18 (accelerometer) — together these give you the architecture
> vocabulary needed to design the project from scratch.

---

## Hardware

- nRF5340 DK (fully featured)
- LED1 (`led0`) — BLE advertising: fast blink (500 ms)
- LED2 (`led1`) — BLE connected: steady on
- LED3 (`led2`) — Sensor error: blinks 3× on error
- LED4 (`led3`) — Heartbeat: 1 Hz blink from sensor thread
- Button 1 (`sw0`) — Force disconnect / reset advertising

Use the **nRF Connect** mobile app or **nRF Connect for Desktop** to scan, connect,
and read the sensor characteristic.

---

## System Architecture

```
┌──────────────────────────────────────────────────────────┐
│  main()                                                    │
│  ├── BleManager         (FSM: Idle→Advertising→Connected) │
│  │   ├── Led led_adv    (LED1)                            │
│  │   └── Led led_conn   (LED2)                            │
│  └── SensorThread       (Thread wrapper, 200 ms period)   │
│      ├── SimTempSensor  (Sensor subclass)                 │
│      ├── RingBuffer<float, 16>  (last 16 samples)         │
│      └── Result<float, SensorError>  (error handling)     │
└──────────────────────────────────────────────────────────┘
```

---

## Functional Requirements

### BLE Advertisement
- Device name: `NRF5340-SENSOR`
- Advertise a custom 128-bit UUID service
- One GATT characteristic: Temperature (float, notify + read)
- Advertising starts automatically on boot
- LED1 blinks at 500 ms while advertising; LED2 stays on while connected

### Sensor Thread
- Runs every 200 ms, reads the simulated temperature sensor
- Stores the last 16 readings in a `RingBuffer<float, 16>`
- On read error, blinks LED3 three times and logs the error name
- If BLE is connected, notifies the temperature characteristic with the latest value

### BLE Manager
- State machine: `Idle → Advertising → Connected → Idle`
- Accepts event callbacks (lambdas) for connect and disconnect
- `on_connect` lambda: turn LED2 on, stop LED1 blinking
- `on_disconnect` lambda: restart advertising, start LED1 blinking again

### Button
- Press Button 1 to force-disconnect the current BLE peer
- If already disconnected, restart advertising

---

## Suggested File Structure

```
src/
├── main.cpp              — wires everything together
├── led.hpp               — Led class (from Ex 02 + 03)
├── led.cpp
├── sensor.hpp            — Sensor base, SimTempSensor, SensorError, Result
├── sensor.cpp
├── ring_buffer.hpp       — RingBuffer<T, N> (from Ex 05, header-only)
├── ble_manager.hpp       — BleManager class + State enum class
├── ble_manager.cpp       — BLE callbacks, advertising setup
└── sensor_thread.hpp/cpp — SensorThread : Thread<2048>
```

---

## Implementation Hints

### BLE in Zephyr C++

The Zephyr BLE callbacks (`bt_conn_cb`, `bt_le_adv_param`) are C structs with
function pointer fields. Use static member functions as the bridge:

```cpp
/* In ble_manager.cpp */
static void on_connected_static(struct bt_conn *conn, uint8_t err) {
    BleManager::instance().on_connected(conn, err);
}
```

A singleton `BleManager::instance()` is acceptable here because there is
exactly one BLE stack per device.

### Advertising data

```cpp
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, /* your UUID bytes */),
};
```

### Notify from C++

```cpp
bt_gatt_notify(conn_, &sensor_svc.attrs[2], &temperature, sizeof(temperature));
```

---

## Build & Flash

```bash
west build -b nrf5340dk/nrf5340/cpuapp
west flash
```

Open a serial terminal at 115200 baud to see the sensor log.
Use **nRF Connect** (mobile or desktop) to connect and read the temperature characteristic.

---

## Assessment Checklist

When you consider yourself done, review this checklist:

- [ ] `Led` class: no raw GPIO calls outside the class
- [ ] `BleManager` has no raw `if(state == 1)` — uses `enum class State`
- [ ] `SensorThread::run()` never calls `k_mutex_unlock()` directly — uses RAII lock
- [ ] `RingBuffer` is used with at least two different `T` types in the project
- [ ] `read_temperature()` returns `Result<float, SensorError>` — never returns `-1`
- [ ] BLE callbacks are registered via lambda captures, not global function pointers
- [ ] All `[[nodiscard]]` returns are checked at every call site

---

## Additional Resources

- [Zephyr BLE Peripheral Sample](https://docs.zephyrproject.org/latest/samples/bluetooth/peripheral/README.html)
- [Zephyr GATT API](https://docs.zephyrproject.org/latest/connectivity/bluetooth/api/gatt.html)
- [nRF Connect app (iOS/Android)](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile)
- [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop)
- nRF5340 DK User Guide (see `materials/` folder)
