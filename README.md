# uartbt

ESP32 UART ↔ Bluetooth Classic (SPP) bridge.  
Bytes received on the UART are forwarded over Bluetooth, and bytes received
over Bluetooth are forwarded to the UART — transparently in both directions.

Low-power features enabled by default:

| Feature | Setting | Effect |
|---|---|---|
| CPU frequency | 80 MHz (down from 240 MHz) | ~3× lower MCU power draw |
| BT TX power | 0 dBm (down from +9 dBm) | Lower radio power, adequate for room-distance use |
| BT modem sleep | enabled | Radio sleeps between packets |

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

All user-adjustable settings live in [`src/config.h`](src/config.h):

```c
// Bluetooth name shown during pairing
#define BT_DEVICE_NAME "ESP32-UART-Bridge"

// TX power: ESP_PWR_LVL_N12 (−12 dBm) … ESP_PWR_LVL_P9 (+9 dBm)
#define BT_TX_POWER ESP_PWR_LVL_N0   // 0 dBm

// UART port and pins
#define BRIDGE_UART        Serial2
#define BRIDGE_UART_BAUD   115200
#define BRIDGE_UART_RX_PIN 16
#define BRIDGE_UART_TX_PIN 17

// CPU frequency in MHz (minimum for stable BT on ESP32 is 80)
#define CPU_FREQ_MHZ 80
```

---

## Build & Flash

This project uses [PlatformIO](https://platformio.org/).

```bash
# Install PlatformIO CLI (once)
pip install platformio

# Build
pio run

# Flash (replace /dev/ttyUSB0 with your port)
pio run --target upload --upload-port /dev/ttyUSB0

# Monitor serial output
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

---

## Usage

1. Flash the firmware.
2. On your phone/PC, scan for Bluetooth devices and pair with **ESP32-UART-Bridge**.
3. Open a Bluetooth Serial terminal (e.g. Serial Bluetooth Terminal on Android, or
   `rfcomm` + `screen` on Linux).
4. Any text sent from the Bluetooth terminal appears on the UART, and vice versa.