#include "ble_manager.hpp"
#include <zephyr/sys/printk.h>

namespace sensor_node {

/* Advertising data — modify UUID bytes to your own 128-bit UUID */
static const struct bt_data kAdvData[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88),
};

static struct bt_conn_cb kConnCallbacks = {
    .connected    = BleManager::connected_cb,
    .disconnected = BleManager::disconnected_cb,
};

BleManager &BleManager::instance()
{
    static BleManager inst;
    return inst;
}

void BleManager::init()
{
    bt_conn_cb_register(&kConnCallbacks);

    int err = bt_enable(nullptr);
    if (err) {
        printk("BLE init failed: %d\n", err);
        return;
    }
    printk("BLE enabled\n");
    state_ = BleState::Idle;
}

void BleManager::start_advertising()
{
    if (state_ != BleState::Idle) {
        return;
    }
    int err = bt_le_adv_start(BT_LE_ADV_CONN_NAME,
                               kAdvData, ARRAY_SIZE(kAdvData),
                               nullptr, 0);
    if (err) {
        printk("Advertising failed: %d\n", err);
        return;
    }
    state_ = BleState::Advertising;
    printk("BLE advertising started\n");
}

void BleManager::disconnect()
{
    if (conn_) {
        bt_conn_disconnect(conn_, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    }
}

void BleManager::notify_temperature(float temp)
{
    /* TODO: call bt_gatt_notify with the temperature characteristic attribute */
    (void)temp;
}

void BleManager::handle_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("BLE connect failed: %u\n", err);
        return;
    }
    conn_  = bt_conn_ref(conn);
    state_ = BleState::Connected;
    printk("BLE connected\n");
    if (on_connect_) on_connect_();
}

void BleManager::handle_disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("BLE disconnected (reason %u)\n", reason);
    if (conn_) {
        bt_conn_unref(conn_);
        conn_ = nullptr;
    }
    state_ = BleState::Idle;
    if (on_disconnect_) on_disconnect_();
}

void BleManager::connected_cb(struct bt_conn *conn, uint8_t err)
{
    instance().handle_connected(conn, err);
}

void BleManager::disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
    instance().handle_disconnected(conn, reason);
}

} // namespace sensor_node
