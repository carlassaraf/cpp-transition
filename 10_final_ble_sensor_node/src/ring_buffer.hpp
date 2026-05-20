#pragma once
#include <array>
#include <cstddef>

namespace sensor_node {

/* Paste your completed RingBuffer from Exercise 05 here.
 * This is the header-only, stack-allocated version. */
template <typename T, size_t N>
class RingBuffer {
public:
    static_assert(N > 0, "RingBuffer size must be greater than zero");

    bool push(const T &item);
    bool pop(T &out);
    bool is_empty() const;
    bool is_full() const;
    size_t size() const { return count_; }
    static constexpr size_t capacity() { return N; }

private:
    std::array<T, N> buf_{};
    size_t head_{0};
    size_t tail_{0};
    size_t count_{0};
};

/* --- Inline implementations (header-only template) --- */

template <typename T, size_t N>
bool RingBuffer<T, N>::push(const T &item)
{
    /* TODO: copy from Exercise 05 */
    return false;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::pop(T &out)
{
    /* TODO: copy from Exercise 05 */
    return false;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::is_empty() const
{
    /* TODO */
    return true;
}

template <typename T, size_t N>
bool RingBuffer<T, N>::is_full() const
{
    /* TODO */
    return false;
}

} // namespace sensor_node
