/*
 * uartbt – ESP32 UART ↔ Bluetooth Classic (SPP) bridge
 *
 * Data flow:
 *   UART RX  →  Bluetooth TX
 *   Bluetooth RX  →  UART TX
 *
 * Low-power measures applied at startup:
 *   1. CPU clock reduced to CPU_FREQ_MHZ (80 MHz).
 *   2. Bluetooth TX power reduced to BT_TX_POWER.
 *   3. Bluetooth modem sleep enabled so the radio sleeps between packets.
 */

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "esp_bt.h"   // esp_bredr_tx_power_set(), esp_bt_sleep_enable()

// ---------------------------------------------------------------------------
// Configuration – edit these values to suit your setup
// ---------------------------------------------------------------------------

// Bluetooth device name (visible during pairing)
#define BT_DEVICE_NAME "ESP32-UART-Bridge"

// Bluetooth TX power: ESP_PWR_LVL_N12 (−12 dBm) … ESP_PWR_LVL_P9 (+9 dBm)
#define BT_TX_POWER ESP_PWR_LVL_N0

// UART bridge port and pins (Serial2 = GPIO 16 RX / GPIO 17 TX on most boards)
#define BRIDGE_UART        Serial2
#define BRIDGE_UART_BAUD   115200
#define BRIDGE_UART_RX_PIN 16
#define BRIDGE_UART_TX_PIN 17

// CPU frequency in MHz (80 is the minimum for stable Bluetooth)
#define CPU_FREQ_MHZ 80

// Max bytes forwarded per loop() iteration per direction
#define BRIDGE_BUF_SIZE 256

static BluetoothSerial SerialBT;

void setup() {
    // -----------------------------------------------------------------------
    // 1. Reduce CPU frequency for lower power consumption.
    //    80 MHz is the minimum that keeps Bluetooth functional on ESP32.
    // -----------------------------------------------------------------------
    setCpuFrequencyMhz(CPU_FREQ_MHZ);

    // -----------------------------------------------------------------------
    // 2. Initialise the hardware UART used as the bridge port.
    // -----------------------------------------------------------------------
    BRIDGE_UART.begin(BRIDGE_UART_BAUD, SERIAL_8N1,
                      BRIDGE_UART_RX_PIN, BRIDGE_UART_TX_PIN);

    // -----------------------------------------------------------------------
    // 3. Start Bluetooth Serial (SPP).
    // -----------------------------------------------------------------------
    SerialBT.begin(BT_DEVICE_NAME);

    // -----------------------------------------------------------------------
    // 4. Reduce Bluetooth TX power.
    //    Both min and max are set to the same value so the stack always uses
    //    the reduced level rather than ramping up to maximum.
    // -----------------------------------------------------------------------
    esp_bredr_tx_power_set(BT_TX_POWER, BT_TX_POWER);

    // -----------------------------------------------------------------------
    // 5. Enable Bluetooth modem sleep so the radio can sleep between packets.
    // -----------------------------------------------------------------------
    esp_bt_sleep_enable();
}

void loop() {
    // -----------------------------------------------------------------------
    // Forward: UART → Bluetooth
    // -----------------------------------------------------------------------
    if (BRIDGE_UART.available()) {
        uint8_t buf[BRIDGE_BUF_SIZE];
        size_t  len = 0;

        while (BRIDGE_UART.available() && len < sizeof(buf)) {
            buf[len++] = static_cast<uint8_t>(BRIDGE_UART.read());
        }

        if (len > 0) {
            SerialBT.write(buf, len);
        }
    }

    // -----------------------------------------------------------------------
    // Forward: Bluetooth → UART
    // -----------------------------------------------------------------------
    if (SerialBT.available()) {
        uint8_t buf[BRIDGE_BUF_SIZE];
        size_t  len = 0;

        while (SerialBT.available() && len < sizeof(buf)) {
            buf[len++] = static_cast<uint8_t>(SerialBT.read());
        }

        if (len > 0) {
            BRIDGE_UART.write(buf, len);
        }
    }
}
