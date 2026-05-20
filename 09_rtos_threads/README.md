# Exercise 09 — C++ RTOS Thread Wrappers

## Goal

Wrap Zephyr's C thread API in a clean C++ class hierarchy.
You will learn the static-member-function trick, how to pass `this` as a thread
argument, and why `std::atomic` is the correct tool for lock-free shared flags.

---

## Concepts Covered

- **Static member function as C callback**: required by the Zephyr `k_thread_create` API
- **Passing `this` to a C callback**: bridge between C and C++ object model
- **Abstract `Thread` base class** with a pure-virtual `run()` method
- **`std::atomic<T>`**: lock-free shared state between threads
- **RAII thread lifecycle**: start in constructor, optional join/abort
- **`K_THREAD_STACK_DEFINE`**: Zephyr's stack allocation macro

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| Wrapping a C HAL API in a C++ class | **Chapter 13** — *RAII and Wrapping C APIs* | [`Chapter13/uart_c_hal_wrapper/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter13/uart_c_hal_wrapper) — the UART wrapper uses the same static-member-function trampoline pattern |
| RAII for resource lifecycle | **Chapter 13** | [`Chapter13/lfs_raii/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter13/lfs_raii) |
| C/C++ interface boundary (calling C from C++) | **Chapter 17** — *C/C++ Interface Boundary (CIB)* | [`Chapter17/cib/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter17/cib) |
| Type safety for thread arguments | **Chapter 9** — *Type Safety* | [`Chapter09/type_safety/app/src/main_reinterpret_cast.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter09/type_safety/app/src) — explains why casting `void *arg` back is valid but needs care |

> **Reading tip:** The `uart_c_hal_wrapper` in Chapter 13 is the closest book analog to
> what you're building. It wraps a C callback API (UART RX interrupt) using a static
> member function and stores state in the class — the same trick the `Thread::entry()`
> static function uses to call `run()` on the right object instance.

---

## Hardware

- nRF5340 DK
- LED1 (`led0`) — blinks at 200 ms (Thread A)
- LED2 (`led1`) — blinks at 700 ms (Thread B)
- LED3 (`led2`) — mirrors a shared atomic flag (Thread C reads it)
- Button 1 (`sw0`) — toggles the shared flag from `main()`

---

## Background: The C Thread Pattern

```c
K_THREAD_STACK_DEFINE(my_stack, 1024);
struct k_thread my_tid;

static void my_thread_fn(void *a, void *b, void *c) { /* ... */ }

k_thread_create(&my_tid, my_stack, K_THREAD_STACK_SIZEOF(my_stack),
                my_thread_fn, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
```

The C API forces the thread body into a standalone function.
The C++ wrapper puts the body in a virtual method, where it has full access to
the object's private members.

---

## The Exercise

### Part A — Abstract `Thread` base class

```cpp
class Thread {
public:
    explicit Thread(size_t stack_size, int priority);
    virtual ~Thread();
    void start();

protected:
    virtual void run() = 0;

private:
    static void entry(void *arg, void *, void *);
    k_thread thread_;
    k_thread_stack_t *stack_;
    size_t stack_size_;
    int priority_;
};
```

Key insight: `entry()` is `static`, so it has no `this`. It receives `arg`,
which is cast back to `Thread *` to call `run()`.

> **Note:** In Zephyr, `K_THREAD_STACK_DEFINE` is a macro that must be called
> at file scope. For dynamic stack allocation use `k_thread_stack_alloc()` with
> `CONFIG_DYNAMIC_THREAD=y`, or use a fixed-size macro with a template parameter.
> The starter code shows the fixed-size approach.

### Part B — Two concrete thread subclasses

```cpp
class LedBlinkThread : public Thread {
public:
    LedBlinkThread(const gpio_dt_spec &led, uint32_t interval_ms, ...);
protected:
    void run() override;
};
```

Each instance blinks its assigned LED at its own interval, independent of the other.

### Part C — Shared atomic flag

```cpp
static std::atomic<bool> g_flag{false};
```

Thread C reads `g_flag` every 50 ms and sets LED3 accordingly.
`main()` toggles `g_flag` when Button 1 is pressed.
No mutex needed — `std::atomic` guarantees visibility across cores/threads.

---

## Build & Flash

```bash
west build -b nrf5340dk/nrf5340/cpuapp
west flash
```

---

## Things to Observe

- Without `std::atomic`, try accessing a plain `bool` from two threads and add
  `printk()` to count how many times the flag is missed. On Cortex-M, reads/writes
  of aligned words are naturally atomic, but the compiler can still reorder them.
  `std::atomic` adds the necessary memory-order barriers.
- The vtable overhead: each `Thread` subclass pays one vtable pointer.
  On a Cortex-M33 that is 4 bytes per instance. Weigh this against the design benefit.
- Compare thread stack usage with `CONFIG_THREAD_ANALYZER=y`.

---

## Bonus Challenges

1. Add a `stop()` method that sets an atomic `running_` flag to `false` and
   the `run()` loop checks it. This is graceful shutdown.
2. Add a `Thread::sleep_ms(uint32_t ms)` protected helper so derived classes
   don't call Zephyr directly.
3. Implement a `PeriodicThread` subclass that calls a virtual `tick()` at a
   fixed interval, handling drift using `k_uptime_get()`.

---

## Additional Resources

- [Zephyr Threads API](https://docs.zephyrproject.org/latest/kernel/services/threads/index.html)
- [cppreference — std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
- [cppreference — Memory order](https://en.cppreference.com/w/cpp/atomic/memory_order)
- [Zephyr Thread Analyzer](https://docs.zephyrproject.org/latest/services/debugging/thread-analyzer.html)
