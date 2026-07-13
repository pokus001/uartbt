#pragma once

// ---------------------------------------------------------------------------
// Bluetooth configuration
// ---------------------------------------------------------------------------

// Name advertised over Bluetooth (visible to pairing devices)
#define BT_DEVICE_NAME "ESP32-UART-Bridge"

// Bluetooth Classic TX power level.
// Reducing this extends battery life and limits range.
// Available levels (esp_power_level_t):
//   ESP_PWR_LVL_N12 (-12 dBm) ... ESP_PWR_LVL_P9 (+9 dBm)
// Default: ESP_PWR_LVL_N0 (0 dBm) – good range inside a room
#define BT_TX_POWER ESP_PWR_LVL_N0

// ---------------------------------------------------------------------------
// UART (bridge port) configuration
// ---------------------------------------------------------------------------

// Hardware serial port to bridge.  Serial2 uses GPIO 16 (RX) / 17 (TX)
// by default on most ESP32 dev boards.
#define BRIDGE_UART        Serial2
#define BRIDGE_UART_BAUD   115200
#define BRIDGE_UART_RX_PIN 16
#define BRIDGE_UART_TX_PIN 17

// ---------------------------------------------------------------------------
// Low-power CPU frequency
// ---------------------------------------------------------------------------

// 80 MHz is the lowest frequency that keeps Bluetooth stable on ESP32.
// Reducing from the default 240 MHz roughly triples battery life of the MCU.
#define CPU_FREQ_MHZ 80

// ---------------------------------------------------------------------------
// Bridge buffer size (bytes)
// ---------------------------------------------------------------------------

// Maximum bytes forwarded in a single loop() iteration per direction.
#define BRIDGE_BUF_SIZE 256
