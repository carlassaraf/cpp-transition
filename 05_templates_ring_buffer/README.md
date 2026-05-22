# Exercise 05 — Templates: Type-Safe Ring Buffer

## Goal

Replace the classic C `void *` ring buffer with a C++ class template.
You will see first-hand how templates give you zero-overhead type safety
and why they are the embedded-friendly alternative to `void *` casts.

---

## Concepts Covered

- **Class templates** (`template <typename T, size_t N>`)
- **Non-type template parameters**: `N` fixes the buffer size at compile time — **no heap allocation**
- **`static_assert`**: catch misuse at compile time instead of runtime
- **`std::array`** as the underlying storage (stack-allocated, bounds-aware)
- **Template instantiation**: the compiler generates separate code for each `<T, N>` combination
- Why templates are preferred over `void *` in embedded: no casts, no wrong-type errors

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| C vs C++ ring buffer — `void *` erasure vs template | **Chapter 1** — *Introduction / C vs C++* | [`Chapter01/ring_buffer.c`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/ring_buffer.c) (C), [`Chapter01/ring_buffer.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/ring_buffer.cpp) (C++), [`Chapter01/ring_buffer_type_erasure.c`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/ring_buffer_type_erasure.c), [`Chapter01/ring_buffer_compile_time_generic.c`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/ring_buffer_compile_time_generic.c) |
| Template functions, specialization, CRTP | **Chapter 8** — *Templates and Generic Programming* | [`Chapter08/template_function.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter08/template_function.cpp), [`Chapter08/template_specialization.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter08/template_specialization.cpp) |
| `constexpr` for compile-time sizes | **Chapter 1** | [`Chapter01/compile_time_const.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/blob/main/Chapter01/compile_time_const.cpp) |
| Compile-time lookup tables | **Chapter 11** — *Compile-Time Computation* | [`Chapter11/compile_time/app/src/main_lookup_table.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter11/compile_time/app/src) |

> **Reading tip:** Start with the four ring-buffer files in Chapter 1 — read them
> in this order: `.c` → `_type_erasure.c` → `_compile_time_generic.c` → `.cpp`.
> You will see exactly the progression from C to C++ that this exercise asks you to implement.
> Then read `Chapter08/template_function.cpp` for the general template syntax.

---

## How a Ring Buffer Works

A ring buffer is a fixed-size array treated as a circle. Two indices track where
to write next (`tail`) and where to read next (`head`). When an index reaches the
end of the array it wraps back to `[0]` — hence "ring". A `count` field
disambiguates the full vs. empty cases (both have `head == tail`).

The examples below use `RingBuffer<char, 6>`.

---

**1 — Initial state (empty)**

```
 ┌────┬────┬────┬────┬────┬────┐
 │    │    │    │    │    │    │
 └────┴────┴────┴────┴────┴────┘
  [0]  [1]  [2]  [3]  [4]  [5]
   ↑
 head = tail = 0    count = 0
```

---

**2 — After `push(A)`, `push(B)`, `push(C)`**

Each push writes at `tail` then advances it.

```
 ┌────┬────┬────┬────┬────┬────┐
 │ A  │ B  │ C  │    │    │    │
 └────┴────┴────┴────┴────┴────┘
  [0]  [1]  [2]  [3]  [4]  [5]
   ↑              ↑
 head            tail            count = 3
```

---

**3 — After `pop() → A`, `pop() → B`**

Each pop reads from `head` then advances it. The slots are logically free but the
data is not erased — only `head` moves forward.

```
 ┌────┬────┬────┬────┬────┬────┐
 │ ·  │ ·  │ C  │    │    │    │
 └────┴────┴────┴────┴────┴────┘
  [0]  [1]  [2]  [3]  [4]  [5]
              ↑    ↑
            head  tail           count = 1
```

---

**4 — After `push(D)`, `push(E)`, `push(F)`, `push(G)`, `push(H)` — wrap-around**

`tail` hits `[5]`, wraps to `[0]`, and continues filling the slots freed earlier
by the two pops. The buffer is now full (`count == N`).

```
 ┌────┬────┬────┬────┬────┬────┐
 │ G  │ H  │ C  │ D  │ E  │ F  │
 └────┴────┴────┴────┴────┴────┘
  [0]  [1]  [2]  [3]  [4]  [5]
              ↑
         head = tail = 2         count = 6  (FULL)
```

Reading order (FIFO): C → D → E → F → G → H

> `head == tail` is true for both the **empty** and **full** states — `count` is the
> only way to tell them apart.

---

**Index arithmetic — the key line in every ring buffer:**

```cpp
tail_ = (tail_ + 1) % N;   // wrap [N-1] back to [0]
head_ = (head_ + 1) % N;
```

---

## Hardware

- nRF5340 DK
- No GPIO needed — this is a pure software exercise
- UART output to verify buffer behaviour

---

## The Exercise

### Part A — Implement `RingBuffer<T, N>`

```cpp
template <typename T, size_t N>
class RingBuffer {
public:
    bool push(const T &item);     // returns false when full
    bool pop(T &out);             // returns false when empty
    bool is_empty() const;
    bool is_full() const;
    size_t size() const;
    static constexpr size_t capacity() { return N; }

private:
    std::array<T, N> buf_;
    size_t head_{0};
    size_t tail_{0};
    size_t count_{0};
};
```

Rules:
- **No dynamic allocation** — `buf_` is a `std::array`, allocated on the stack or as a global.
- `push()` must not block; return `false` if full.
- `pop()` must not block; return `false` if empty.

### Part B — Use it with three different types

```cpp
RingBuffer<uint8_t, 64>  uart_rx_buf;
RingBuffer<float, 16>    adc_samples;
RingBuffer<uint32_t, 8>  event_queue;
```

Write a test loop that pushes a known sequence into each buffer and
pops it back out, printing mismatches with `printk()`.

### Part C — `static_assert`

Add a `static_assert` to reject a buffer of size 0:

```cpp
static_assert(N > 0, "RingBuffer size must be greater than zero");
```

Try instantiating `RingBuffer<int, 0>` and observe the error message.

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

## C vs C++ Comparison

| C approach | C++ template approach |
|---|---|
| `void ring_push(ring_t *r, void *item, size_t sz)` | `buf.push(item)` |
| Cast back to `uint8_t *` at every call site | Compiler enforces the type |
| Wrong-type bugs are silent | Wrong type = compile error |
| Size carried at runtime (extra RAM) | `N` compiled in, zero runtime overhead |

---

## Things to Observe

- Check the `.map` file: `RingBuffer<uint8_t, 64>` and `RingBuffer<float, 16>` are
  separate instantiations. This is *code bloat* vs *type safety*. In practice for
  embedded, use a small number of distinct instantiations.
- Try using `sizeof(RingBuffer<uint8_t, 64>)` — it is exactly 64 + overhead for
  head/tail/count. No hidden heap pointer.

---

## Bonus Challenges

1. Add a `peek()` method that reads without consuming.
2. Add an iterator so the buffer works in a range-based `for` loop.
3. Make the buffer thread-safe by adding a `k_spinlock` (preview of Exercise 09).
4. Compare your implementation with Zephyr's built-in `RING_BUF` C macro API —
   count the lines of code and the type-safety gaps.

---

## Additional Resources

- [cppreference — Class templates](https://en.cppreference.com/w/cpp/language/class_template)
- [cppreference — std::array](https://en.cppreference.com/w/cpp/container/array)
- [cppreference — static_assert](https://en.cppreference.com/w/cpp/language/static_assert)
- [Zephyr Ring Buffer API](https://docs.zephyrproject.org/latest/kernel/data_structures/ring_buffers.html)
