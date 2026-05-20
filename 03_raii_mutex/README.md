# Exercise 03 — RAII: Resource Acquisition Is Initialization

## Goal

Understand the most important C++ idiom for embedded firmware: **RAII**.
When a resource (mutex, GPIO pin, semaphore) is tied to an object's lifetime,
the compiler guarantees cleanup even when you return early, throw (if enabled),
or simply forget.

---

## Concepts Covered

- **Destructor**: called automatically when an object goes out of scope
- **RAII pattern**: acquire in constructor, release in destructor
- **`ScopedLock`**: a mutex guard whose destructor calls `k_mutex_unlock()`
- **Why early-return is safe with RAII** vs the C `goto cleanup` pattern
- **`= delete`**: preventing copies of non-copyable resources

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| RAII for file-system resources | **Chapter 13** — *RAII and Wrapping C APIs* | [`Chapter13/lfs_raii/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter13/lfs_raii) — RAII applied to LittleFS, directly analogous to your mutex wrapper |
| Wrapping a C HAL in C++ with RAII | **Chapter 13** | [`Chapter13/uart_c_hal_wrapper/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter13/uart_c_hal_wrapper) |
| Memory management, `new`/`delete`, terminate handlers | **Chapter 2** — *Memory Management* | [`Chapter02/terminate_handler.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter02/terminate_handler.cpp), [`Chapter02/new_deleted.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter02/new_deleted.cpp) |

> **Reading tip:** Read the `lfs_raii` example first — it is the clearest book demonstration
> of the acquire-in-constructor / release-in-destructor pattern you are implementing here.
> The C HAL wrapper example shows the same idea applied to UART, which is very close to
> wrapping `k_mutex`.

---

## Hardware

- nRF5340 DK
- LED1 (`led0`) — controlled by Thread A
- LED2 (`led1`) — controlled by Thread B
- Both threads share a counter protected by a mutex

---

## Background: The C Problem

In C, every early-return path requires manual cleanup:

```c
int do_work(void) {
    k_mutex_lock(&my_mutex, K_FOREVER);

    if (condition_a()) {
        k_mutex_unlock(&my_mutex);  /* easy to forget */
        return -EINVAL;
    }
    if (condition_b()) {
        k_mutex_unlock(&my_mutex);  /* copy-paste error waiting to happen */
        return -EBUSY;
    }

    do_the_thing();
    k_mutex_unlock(&my_mutex);
    return 0;
}
```

---

## The Exercise

### Part A — Implement `ScopedLock`

```cpp
class ScopedLock {
public:
    explicit ScopedLock(k_mutex &mutex);  // locks the mutex
    ~ScopedLock();                         // unlocks the mutex
    // TODO: delete copy constructor and copy-assignment operator
private:
    k_mutex &mutex_;
};
```

### Part B — Implement `SharedCounter`

A class that wraps an `int` and a `k_mutex`. Provide:
- `void increment()` — locks with a `ScopedLock`, increments, returns (lock released automatically)
- `int get() const` — locks with a `ScopedLock`, reads value, returns

The key point: **neither function calls `k_mutex_unlock()` explicitly**.

### Part C — Two threads

Thread A increments the counter every 200 ms and blinks LED1.
Thread B reads the counter every 500 ms and blinks LED2 once per 1000 increments.

Intentionally delete the `ScopedLock` destructor body and observe the
race condition: LED2 will miscount.

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

- Add `printk()` calls inside the lock/unlock to trace the sequence.
- Try making `ScopedLock` copyable. What breaks? Why is that dangerous?
- Notice that `std::lock_guard<std::mutex>` in the STL is exactly this pattern —
  you've just built it yourself for Zephyr's `k_mutex`.

---

## Bonus Challenges

1. Implement a `ScopedGpio` class: constructor calls `gpio_pin_configure_dt()` and
   sets pin high; destructor sets pin low. Useful for a "busy" indicator LED.
2. Add a timeout to `ScopedLock`: if `k_mutex_lock()` times out, set a `locked_`
   flag to `false` and make the destructor check it before unlocking.
3. Look up `std::lock_guard` and `std::unique_lock` — compare their design with yours.

---

## Additional Resources

- [cppreference — RAII](https://en.cppreference.com/w/cpp/language/raii)
- [cppreference — Destructors](https://en.cppreference.com/w/cpp/language/destructor)
- [Zephyr Mutex API](https://docs.zephyrproject.org/latest/kernel/services/synchronization/mutexes.html)
- [cppreference — std::lock_guard](https://en.cppreference.com/w/cpp/thread/lock_guard)
