# Exercise 07 — State Machine with `enum class`

## Goal

Replace a typical C `switch(state)` flag machine with an OOP-based state machine
that makes invalid transitions impossible to express and self-documents the protocol.

---

## Concepts Covered

- **`enum class`** (scoped enums): prevent accidental integer comparisons and name collisions
- **State machine design**: table-driven vs method-per-state
- **Preventing invalid transitions**: `assert()` or returning an error on illegal events
- **`[[nodiscard]]`**: warn when the caller ignores a transition result
- **LED patterns as state indicators**: visual feedback for debugging firmware

## Book Reference

| Topic | Chapter | Book code examples |
|---|---|---|
| Finite State Machine pattern in C++ | **Chapter 16** — *Finite State Machine (FSM)* | [`Chapter16/fsm/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter16/fsm) — the book's full FSM implementation; study the app/src folder |
| Type safety with scoped enums | **Chapter 9** — *Type Safety* | [`Chapter09/type_safety/app/src/main_strong_types.cpp`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter09/type_safety/app/src) |
| `[[nodiscard]]` and attributes | **Chapter 7** — *Error Handling* | [`Chapter07/error_handling/app/src/`](https://github.com/PacktPublishing/Cpp-in-Embedded-Systems/tree/main/Chapter07/error_handling/app/src) |

> **Reading tip:** The Chapter 16 FSM example is the direct model for your `ConnectionFsm`.
> Read it in full before designing your transition table — pay attention to how
> states and events are represented and how transitions are guarded.

---

## Hardware

- nRF5340 DK
- Button 1 (`sw0`) — triggers "connect" event
- Button 2 (`sw1`) — triggers "disconnect" event
- LED1 (`led0`) — steady on = CONNECTED
- LED2 (`led1`) — fast blink = CONNECTING
- LED3 (`led2`) — slow blink = ADVERTISING
- LED4 (`led3`) — off = IDLE

---

## Background: The C Problem

```c
/* C state machine — easy to introduce bugs */
#define STATE_IDLE        0
#define STATE_ADVERTISING 1
#define STATE_CONNECTING  2
#define STATE_CONNECTED   3

int state = STATE_IDLE;

void handle_event(int event) {
    if (state == STATE_IDLE && event == EVENT_START) {
        state = STATE_ADVERTISING;           /* valid */
    } else if (state == STATE_CONNECTED && event == EVENT_START) {
        state = STATE_ADVERTISING;           /* oops — should be invalid */
    }
    /* Nothing stops state = 99; */
}
```

---

## State Diagram

```
  IDLE ──[start]──► ADVERTISING ──[peer found]──► CONNECTING ──[done]──► CONNECTED
   ▲                     │                              │                     │
   └─────────────────────┴──────────[timeout/disconnect]┘                    │
   └────────────────────────────────────────────────────[disconnect]──────────┘
```

---

## The Exercise

### Part A — Define events and states with `enum class`

```cpp
enum class State  { Idle, Advertising, Connecting, Connected };
enum class Event  { Start, PeerFound, ConnectionDone, Disconnect, Timeout };
```

### Part B — Implement `ConnectionFsm`

```cpp
class ConnectionFsm {
public:
    ConnectionFsm();
    [[nodiscard]] bool handle(Event event);   // returns false for invalid transitions
    State current_state() const;
    const char *state_name() const;

private:
    State state_;
    void enter_state(State next);
};
```

`handle()` encodes the valid transition table. For each state, only a specific
subset of events are legal. Return `false` (or `assert`) for anything else.

### Part C — LED feedback

Create an `update_leds()` function that maps each `State` to a distinct LED pattern.
Call it inside `enter_state()`.

### Part D — Button-driven events

Poll Button 1 to fire `Event::Start` (or `Event::PeerFound` / `Event::ConnectionDone`
in sequence) and Button 2 to fire `Event::Disconnect`.

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

- Try writing `state_ = static_cast<State>(99)` — the type system allows it
  (it's a cast), but `handle()` will reject the unknown state. This is why
  `assert(false)` in the default branch of your switch is important.
- Compare the C `#define` approach: `state = 1` is undetectable nonsense;
  `state_ = State::Advertising` is self-documenting and type-checked.
- A `[[nodiscard]]` `handle()` warns you at compile time if you forget to check
  whether the transition was valid.

---

## Bonus Challenges

1. Add a `timeout` counter: if `ADVERTISING` for more than 10 seconds with no
   `PeerFound`, automatically transition to `Idle`.
2. Add a history: store the last 8 state transitions in a `RingBuffer<State, 8>`
   (from Exercise 05) and print it on disconnect.
3. Implement the "state object" pattern: give each state its own class with
   `enter()`, `handle(Event)`, and `exit()` methods.

---

## Additional Resources

- [cppreference — Scoped enumerations](https://en.cppreference.com/w/cpp/language/enum)
- [cppreference — nodiscard](https://en.cppreference.com/w/cpp/language/attributes/nodiscard)
- *UML State Machine diagram notation* — useful for documenting firmware protocols
