# psx-ble
PSX Controller to BLE Adapter

Required libraries:

* ESP32-BLE-Gamepad - https://github.com/lemmingDev/ESP32-BLE-Gamepad
* NimBLE - https://github.com/h2zero/NimBLE-Arduino
* PsxNewLib - https://github.com/SukkoPera/PsxNewLib
* DigitalIO - https://github.com/greiman/DigitalIO
* Battery18650Stats - https://github.com/danilopinotti/Battery18650Stats

Wiring:
```
PSX - Pin Description - ESP32
1   - Data (MISO)     - 19 (VSPI_MISO) - Pulled to 3.3v with a 1k resistor
2   - Command (MOSI)  - 23 (VSPI_MOSI) - Pulled to 3.3v with a 1k resistor
3   - Rumble 7v to 9v - No Connection
4   - Ground          - Ground
5   - 3.3v Power      - 3.3v
6   - Attention       - 5  (VSPI_CS)
7   - Clock (SCK)     - 18 (VSPI_CLK)
8   - Unused          - No Connection
9   - Acknowledge     - No Connection
```

I had to add an additional 3.3v regulator for an SCPH-1110 dual flight stick controller, the one on my ESP32 devkit couldn't power the controller and the ESP32. It did handle a normal controller though.

Touch line:  Pin 4 on ESP32

Touch line uses a capacitive sensor to detect touch. Wire it to something metallic to use as a button.
Touch to wake from sleep, hold for 1s to reboot (reinitialize PSX, reconnect BT)

Battery Voltage line: Pin 35 on ESP32

Battery Voltage line wiring:

Battery + <> 47k Resistor <> Battery Voltage Line <> 47k Resistor <> Battery -

ESP32 will go into deep sleep when PSX controller is disconnected or after 10 minutes of no button presses on the PSX controller.

For best battery life when not in use disconnect PSX controller.
