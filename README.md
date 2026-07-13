# uartbt

ESP32 UART ↔ Bluetooth Classic (SPP) bridge.  
Bytes received on the UART are forwarded over Bluetooth, and bytes received
over Bluetooth are forwarded to the UART — transparently in both directions.

Built with **ESP-IDF** (plain C, single source file).

Low-power features enabled by default:

| Feature | Setting | Effect |
|---|---|---|
| CPU frequency | 80 MHz (down from 240 MHz) | ~3× lower MCU power draw |
| BT TX power | 0 dBm (down from +9 dBm) | Lower radio power, adequate for room-distance use |
| BT modem sleep | enabled | Radio sleeps between packets |
| BLE memory | released | Frees ~30 KB of RAM |

---

## Hardware

| Signal | ESP32 pin |
|--------|-----------|
| UART RX | GPIO 16 |
| UART TX | GPIO 17 |
| GND | GND |

Connect the external device's **TX → ESP32 GPIO 16** and **RX → ESP32 GPIO 17**.

---

## Configuration

All user-adjustable settings are `#define`s at the top of [`main/src/main.c`](main/src/main.c):

```c
#define BT_DEVICE_NAME      "ESP32-UART-Bridge"
#define BT_TX_POWER         ESP_PWR_LVL_N0       // 0 dBm
#define BRIDGE_UART_NUM     UART_NUM_2
#define BRIDGE_UART_BAUD    115200
#define BRIDGE_UART_TX_PIN  17
#define BRIDGE_UART_RX_PIN  16
```

CPU frequency is set in `sdkconfig.defaults` (`CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_80=y`).

---

## Build & Flash (ESP-IDF)

Requires [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/).

```bash
# Set up ESP-IDF environment (once per terminal session)
. $IDF_PATH/export.sh

# Build
idf.py build

# Flash (replace /dev/ttyUSB0 with your port)
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor
```

---

## Usage

1. Flash the firmware.
2. On your phone/PC, scan for Bluetooth devices and pair with **ESP32-UART-Bridge**.
3. Open a Bluetooth Serial terminal (e.g. Serial Bluetooth Terminal on Android, or
   `rfcomm` + `screen` on Linux).
4. Any text sent from the Bluetooth terminal appears on the UART, and vice versa.