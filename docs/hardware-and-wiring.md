# Hardware and Wiring

## Components

- PIC18F45K50 microcontroller
- Arduino board
- 16x2 character LCD
- 11 LEDs with current-limiting resistors
- Webcam
- USB connection for the host computer
- UART connection between the PIC and Arduino

## Interfaces

| Connection | Description |
|---|---|
| Computer USB | Connects the Python application to the PIC USB CDC interface |
| PIC UART TX | Connects to the Arduino software serial RX pin (A0) |
| PIC UART ground | Must share a common ground with the Arduino |
| Arduino digital pins 2–12 | Drive the 11 LED outputs through appropriate current-limiting resistors |
| PIC LCD interface | LCD control and data lines are connected to the PIC according to the circuit design |

## Electrical Notes

Verify the supply voltage, logic levels, current-limiting resistors, and pin assignments against the actual hardware before reproducing the circuit.

The Arduino sketch defines A0 as the software serial RX pin and A1 as TX. The current sketch receives commands from the PIC and uses the USB serial interface for debugging. A return UART connection from Arduino TX to PIC RX is not required by this receive-only implementation unless additional functionality uses it.

