# ps224ble
Convert PS2 SPI controller to virtual PS4-style BLE controller

# How it works?
Gamepad data is obtained from PS2 SPI Gamepad/Receiver, then converted to appropriate PS4 signal, and finally send to your device through BLE as a remote gamepad.

# What works and what not?
- [x] Left and right joysticks
- [x] L3, R3 buttons
- [x] 4 buttons on the right (pink, blue, red, green)
- [x] D-Pad. Support all 9 states
- [x] L1, L2, R1, R2
- [x] Special buttons: select/start are mapped to share and option, respectively.
- [ ] L2, R2 as analog buttons (too lazy to do since cheap Chinese clones, including mine, don't support analog)
- [ ] Rumble motors (`ESP32-BLE-Gamepad` does not support)

# Todo?
- Add support for analog buttons
- Make the BLE device detected as PS4 gamepad. For now it's just detected as an `Unknown gamepad`

# Dependencies
- [`PS2X_lib`](https://github.com/madsci1016/Arduino-PS2X)
- [`ESP32-BLE-Gamepad`](https://github.com/lemmingDev/ESP32-BLE-Gamepad)

# License
MIT
