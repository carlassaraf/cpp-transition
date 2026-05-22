/*
 * Exercise 05 — Templates: Type-Safe Ring Buffer
 *
 * Implement RingBuffer<T, N> and verify it with three different types.
 *
 * Build: west build -b nrf5340dk/nrf5340/cpuapp
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <array>
#include <cstddef>

/* =========================================================================
 * TODO: Implement RingBuffer<T, N>
 *
 * Requirements:
 *  - No dynamic memory allocation
 *  - push() returns false when full (non-blocking)
 *  - pop() returns false when empty (non-blocking)
 *  - static_assert rejects N == 0
 * ========================================================================= */
template <typename T, size_t N>
class RingBuffer {
public:
    static_assert(N > 0, "RingBuffer size must be greater than zero");

    bool push(const T &item) {
        // Check buffer item count
        if(is_full()) {
            return false;
        }
        // Copy to tail position and advance tail
        buf_.at(tail_) = item;
        tail_ = (tail_ + 1) % N;
        count_++;
        return true;
    }

    bool pop(T &out) {
        if(is_empty()) {
            return false;
        }
        out = buf_.at(head_);
        head_ = (head_ + 1) % N;
        count_--;
        return true;
    }

    bool peek(T &out) {
        if(is_empty()) {
            return false;
        }
        out = buf_.at(head_);
        return true;
    }

    bool is_empty() const {
        return size() == 0;
    }

    bool is_full() const {
        return size() >= N;
    }

    size_t size() const {
        return count_;
    }

    static constexpr size_t capacity() {
        return N;
    }

private:
    std::array<T, N> buf_;
    size_t head_{0};
    size_t tail_{0};
    size_t count_{0};
};

/* =========================================================================
 * Simple test helper — prints PASS/FAIL
 * ========================================================================= */
static int g_tests_run    = 0;
static int g_tests_failed = 0;

static void check(bool condition, const char *label)
{
    ++g_tests_run;
    if (!condition) {
        ++g_tests_failed;
        printk("FAIL: %s\n", label);
    } else {
        printk("PASS: %s\n", label);
    }
}

int main(void)
{
    printk("=== RingBuffer Tests ===\n");

    /* --- uint8_t buffer --- */
    RingBuffer<uint8_t, 4> byte_buf;

    check(byte_buf.is_empty(),                  "empty on init");
    check(byte_buf.push(0xAA),                  "push 0xAA");
    check(byte_buf.push(0xBB),                  "push 0xBB");
    check(byte_buf.push(0xCC),                  "push 0xCC");
    check(byte_buf.push(0xDD),                  "push 0xDD");
    check(byte_buf.is_full(),                   "full after 4 pushes");
    check(!byte_buf.push(0xEE),                 "push fails when full");

    uint8_t b = 0;
    check(byte_buf.pop(b) && b == 0xAA,         "pop 0xAA (FIFO)");
    check(byte_buf.pop(b) && b == 0xBB,         "pop 0xBB");

    /* --- float buffer --- */
    RingBuffer<float, 8> float_buf;

    for (int i = 0; i < 8; ++i) {
        float_buf.push(static_cast<float>(i) * 1.5f);
    }
    check(float_buf.is_full(),                  "float buf full");

    float f = 0.0f;
    check(float_buf.pop(f) && f == 0.0f,        "float pop 0.0");
    check(float_buf.pop(f) && f == 1.5f,        "float pop 1.5");

    /* --- empty pop --- */
    RingBuffer<uint32_t, 2> tiny;
    uint32_t u = 0;
    check(!tiny.pop(u),                         "pop empty returns false");

    printk("\n%d/%d tests passed\n", g_tests_run - g_tests_failed, g_tests_run);
    return 0;
}
