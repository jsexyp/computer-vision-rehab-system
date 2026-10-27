# System Architecture

## Overview

The system integrates a Python-based computer vision application with a PIC18F45K50 microcontroller and an Arduino-based LED controller.

## Data Flow

1. The webcam captures video frames.
2. The Python application detects body landmarks and evaluates predefined posture conditions.
3. The application transmits posture-related status data to the PIC through USB CDC.
4. The PIC displays status messages on a 16x2 LCD and generates an LED control command.
5. The Arduino receives the command through UART and updates the 11 LED outputs.

## Communication Interfaces

| Interface | Sender | Receiver | Purpose |
|---|---|---|---|
| USB CDC | Python application | PIC18F45K50 | Transmit posture status |
| UART | PIC18F45K50 | Arduino | Transmit LED state pattern |
| GPIO | Arduino | LED board | Control individual LEDs |
| GPIO | PIC18F45K50 | LCD | Display status messages |

The serial interfaces are configured for 9600 baud in the current prototype.

