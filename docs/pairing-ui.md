# Pairing UI and control protocol

## Offline UI

The `PICO-PAIR` mass-storage volume contains:

- `INDEX.HTM` — self-contained HTML, CSS, and JavaScript
- `README.TXT` — browser and connection instructions

No network resources are used. Open `INDEX.HTM` in Google Chrome or Microsoft
Edge, click **Connect to Pico**, and select `BLE to USB HID Bridge`.

Web Serial requires a user gesture and device selection. The page cannot
auto-open or silently access the serial port. Safari and Firefox are not
supported.

## Serial protocol

The CDC interface accepts newline-terminated ASCII commands at 115200 baud.
USB CDC ignores the physical baud rate, but 115200 keeps terminal instructions
consistent.

### Host to Pico

| Command | Effect |
|---|---|
| `STATUS` | Send the current bridge state |
| `PAIR_NEW` | Forget the active keyboard and begin replacement pairing |

The browser presents a confirmation before sending `PAIR_NEW`. A serial
terminal does not, so treat that command as destructive.

### Pico to host

Messages are newline-delimited JSON:

```json
{"type":"hello","protocol":1}
{"type":"status","state":"scanning"}
{"type":"status","state":"pairing","passkey":"123456"}
{"type":"status","state":"ready"}
{"type":"ack","command":"pair_new"}
```

Supported states are `starting`, `scanning`, `connecting`, `pairing`,
`discovering`, `ready`, `disconnected`, and `forgetting`.

## Replacing a keyboard

1. Connect the browser UI.
2. Click **Pair new keyboard** and confirm.
3. Put only the intended keyboard into pairing mode.
4. Type the displayed passkey on that keyboard and press Enter.
5. Wait for **Keyboard ready**.

If the replacement uses a different HID report descriptor, all USB functions
briefly re-enumerate. Reopen the page and reconnect Web Serial if further UI
access is needed.
