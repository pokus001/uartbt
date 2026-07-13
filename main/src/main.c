/*
 * uartbt – ESP32 UART ↔ Bluetooth Classic (SPP) bridge
 *
 * Data flow:
 *   UART RX  →  Bluetooth TX
 *   Bluetooth RX  →  UART TX
 *
 * Low-power measures:
 *   1. CPU clock set to 80 MHz (via menuconfig or sdkconfig).
 *   2. Bluetooth TX power reduced to 0 dBm.
 *   3. Bluetooth modem sleep enabled.
 *
 * Built with ESP-IDF (plain C, no Arduino).
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_pm.h"

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

#define BT_DEVICE_NAME      "ESP32-UART-Bridge"
#define BT_TX_POWER         ESP_PWR_LVL_N0       // 0 dBm

#define BRIDGE_UART_NUM     UART_NUM_2
#define BRIDGE_UART_BAUD    115200
#define BRIDGE_UART_TX_PIN  17
#define BRIDGE_UART_RX_PIN  16
#define UART_BUF_SIZE       1024

#define SPP_SERVER_NAME     "SPP_SERVER"

static const char *TAG = "uartbt";

// Handle of the active SPP connection (0 = not connected)
static uint32_t spp_handle = 0;

// ---------------------------------------------------------------------------
// UART → Bluetooth task
// ---------------------------------------------------------------------------

static void uart_to_bt_task(void *arg)
{
    uint8_t buf[UART_BUF_SIZE];

    while (1) {
        int len = uart_read_bytes(BRIDGE_UART_NUM, buf, sizeof(buf),
                                  pdMS_TO_TICKS(20));
        if (len > 0 && spp_handle != 0) {
            esp_spp_write(spp_handle, len, buf);
        }
    }
}

// ---------------------------------------------------------------------------
// SPP callback – Bluetooth → UART
// ---------------------------------------------------------------------------

static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "SPP initialised");
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0,
                          SPP_SERVER_NAME);
        break;

    case ESP_SPP_START_EVT:
        ESP_LOGI(TAG, "SPP server started");
        esp_bt_dev_set_device_name(BT_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;

    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "SPP client connected");
        spp_handle = param->srv_open.handle;
        break;

    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "SPP connection closed");
        spp_handle = 0;
        break;

    case ESP_SPP_DATA_IND_EVT:
        /* Forward received Bluetooth data to UART */
        if (param->data_ind.len > 0) {
            uart_write_bytes(BRIDGE_UART_NUM,
                             (const char *)param->data_ind.data,
                             param->data_ind.len);
        }
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// Initialisation helpers
// ---------------------------------------------------------------------------

static void init_uart(void)
{
    const uart_config_t uart_cfg = {
        .baud_rate  = BRIDGE_UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(BRIDGE_UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(BRIDGE_UART_NUM, &uart_cfg);
    uart_set_pin(BRIDGE_UART_NUM,
                 BRIDGE_UART_TX_PIN, BRIDGE_UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void init_bluetooth(void)
{
    /* Release BLE memory – we only use Classic BT */
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);

    esp_bluedroid_init();
    esp_bluedroid_enable();

    /* Reduce TX power */
    esp_bredr_tx_power_set(BT_TX_POWER, BT_TX_POWER);

    /* Enable modem sleep for lower power between packets */
    esp_bt_sleep_enable();

    /* Register SPP callback and start SPP */
    esp_spp_register_callback(spp_callback);

    esp_spp_cfg_t spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
    };
    esp_spp_enhanced_init(&spp_cfg);
}

// ---------------------------------------------------------------------------
// app_main – entry point
// ---------------------------------------------------------------------------

void app_main(void)
{
    /* NVS is required by the Bluetooth stack */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    init_uart();
    init_bluetooth();

    /* Start UART → BT forwarding task */
    xTaskCreate(uart_to_bt_task, "uart2bt", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "UART <-> Bluetooth bridge running");
}
