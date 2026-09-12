# Third-party notices

This project builds on open-source work from the following projects and
contributors. The full combined license text distributed with this repository
is in [`LICENSE.TXT`](LICENSE.TXT).

## Pico W BLE to USB HID Bridge

- Project: [shiomachisoft/picow_ble_usb_hid_bridge](https://github.com/shiomachisoft/picow_ble_usb_hid_bridge)
- Author: Shiomachi Software
- Role: Original BLE HID host, USB HID forwarding, persistent target storage,
  and dual-core firmware architecture
- License: See the project-specific and bundled notices in `LICENSE.TXT`

The upstream project also credits
[mateibarbu19](https://github.com/mateibarbu19) for the Pico 2 W support and
stability work released in its 2026-08-10 update.

## BTstack

- Project: [BlueKitchen BTstack](https://github.com/bluekitchen/btstack)
- Copyright: BlueKitchen GmbH and contributors
- Role: Bluetooth Low Energy central, Security Manager, GATT, and HID-over-GATT
  client
- License: BlueKitchen's BTstack Raspberry Pi supplemental license and the
  BTstack example license reproduced in `LICENSE.TXT`

The included BTstack example license restricts redistribution, use, and
modification to personal, non-commercial purposes. Contact BlueKitchen for
commercial licensing.

## TinyUSB

- Project: [hathach/tinyusb](https://github.com/hathach/tinyusb)
- Copyright: Ha Thach and contributors
- Role: USB HID, CDC ACM, and mass-storage device classes
- License: MIT, reproduced in `LICENSE.TXT`

## Raspberry Pi Pico SDK

- Project: [raspberrypi/pico-sdk](https://github.com/raspberrypi/pico-sdk)
- Copyright: Raspberry Pi Ltd and contributors
- Role: RP2040/RP2350 platform support, CYW43 integration, multicore runtime,
  flash coordination, and build tooling
- License: BSD-3-Clause and component-specific licenses included with the SDK

## Arm GNU Toolchain

- Project: [Arm GNU Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain)
- Role: Cross-compiles the firmware for Arm Cortex-M targets
- License: GCC Runtime Library Exception and component-specific GNU licenses

No third-party code is loaded by the offline pairing page at runtime.
