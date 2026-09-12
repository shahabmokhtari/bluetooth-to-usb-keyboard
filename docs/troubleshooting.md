# Troubleshooting

## LED signal interpreting

| LED       | Meaning                                                 |
|-----------|---------------------------------------------------------|
| Off       | Core 1 has not finished starting the wireless chip.     |
| Blinking  | Scanning for a device, or reconnecting to a bonded one. |
| Steady on | Discovery finished; input is being forwarded.           |

## Reading the logs

Pairing codes are shown by `INDEX.HTM` on the read-only `PICO-PAIR` USB drive.
Open it in Google Chrome or Microsoft Edge, click **Connect to Pico**, then
select `BLE to USB HID Bridge`. Firefox and Safari do not support Web Serial.

The same status is available as newline-delimited JSON over the
`BLE to USB HID Bridge` serial port at 115200 baud. Send `STATUS` to request
the current state. Send `PAIR_NEW` to forget the active keyboard and scan for
a replacement.

Logs go to UART0 — GPIO 0 (TX, physical pin 1) and GPIO 1 (RX, pin 2) — so you
need a USB-to-serial adapter, with its ground tied to a ground pin on the board.
The firmware only prints, so wiring the adapter's RX to GPIO 0 and the grounds
together is enough.
(Leaving the adapter's TX disconnected also avoids driving 5 V into a
3.3 V input.)

Both cores share the console, so each line is tagged with the subsystem that
wrote it: `[SYS]`, `[BLE]` or `[USB]`. Some receivers cannot keep up with the
default 115200 — a bit-banged software UART on an AVR, for instance, is
unreliable much above 38400.
So, build with a matching [`UART_BAUD_RATE`, say 9600](building.md).

## A keyboard pairs, then nothing happens

Pairing succeeds, `[BLE] Search for HID service.` is printed, and nothing
follows. The keyboard keeps flashing its pairing light, eventually sleeps, and
the link drops about 30 seconds later.

BTstack normally discovers the Client Characteristic Configuration (CCC)
descriptor with a shortcut that assumes the CCC is the last descriptor of a
characteristic. Keyboards that place a Report Reference after it — which is
common — leave the state machine unable to send the write that enables
notifications, so discovery never finishes and the security manager times out.

The firmware defines `ENABLE_GATT_LEGACY_CCC_DISCOVERY` in `btstack_config.h` to
select the older two-step discovery instead. It costs one extra round trip and
copes with descriptors in any order.

## The bridge goes quiet shortly after power-on

`[SYS] Launching the BLE host on Core 1` is printed and the board then stops
responding, sometimes before the LED starts blinking. The Pico 2 W hits this
more often than the Pico W.

The LED is wired to the CYW43 wireless chip rather than to a GPIO, so Core 0's
LED task and Core 1's `cyw43_arch_init()` both reach for the same SPI bus during
the first milliseconds after boot. If Core 0 gets there first the transfer never
completes and it hangs, taking USB down with it.

`ble_bridge_bt_example_init()` sets `g_cyw43_initialized` once the chip is up,
and `led_blinking_task()` does nothing until it sees that flag.

## The keyboard re-appears on the PC when the BLE link comes up

Expected after initial pairing, or after replacing the keyboard with one that
uses a different HID report descriptor. The bridge disconnects and reconnects
USB once so the PC reads the new descriptor. Normal BLE reconnects do not
restart or re-enumerate USB.
