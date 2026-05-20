#pragma once
#include <functional>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

namespace sensor_node {

enum class BleState { Idle, Advertising, Connected };

class BleManager {
public:
    using ConnectCallback    = std::function<void()>;
    using DisconnectCallback = std::function<void()>;

    static BleManager &instance();

    void init();
    void start_advertising();
    void disconnect();
    void notify_temperature(float temp);

    BleState state() const { return state_; }

    void set_on_connect(ConnectCallback cb)    { on_connect_ = cb; }
    void set_on_disconnect(DisconnectCallback cb) { on_disconnect_ = cb; }

private:
    BleManager() = default;

    void handle_connected(struct bt_conn *conn, uint8_t err);
    void handle_disconnected(struct bt_conn *conn, uint8_t reason);

    /* Static trampolines for Zephyr C callbacks */
    static void connected_cb(struct bt_conn *conn, uint8_t err);
    static void disconnected_cb(struct bt_conn *conn, uint8_t reason);

    BleState state_{BleState::Idle};
    struct bt_conn *conn_{nullptr};
    ConnectCallback on_connect_;
    DisconnectCallback on_disconnect_;
};

} // namespace sensor_node
