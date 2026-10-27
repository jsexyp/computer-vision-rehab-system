
#include <SoftwareSerial.h>

// =========================================================
// PROJECT: Real-Time Computer Vision Rehabilitation System
// PURPOSE: Control an 11-LED feedback board using commands
//          received from the PIC microcontroller.
// COMMUNICATION: SoftwareSerial at 9600 baud
// =========================================================

// Software serial connection to the PIC.
// Arduino RX = A0, Arduino TX = A1.
SoftwareSerial picSerial(A0, A1);

// Digital pins connected to the 11 LEDs.
const int leds[11] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Buffer for incoming commands from the PIC.
// Expected format: 'M' followed by 11 LED states.
char buffer[20];

// Tracks the next position available in the input buffer.
byte indexBuffer = 0;

// =========================================================
// FUNCTION: apagarTodos
// PURPOSE: Turn off all LEDs during initialization.
// =========================================================
void apagarTodos() {
  for (byte i = 0; i < 11; i++) {
    digitalWrite(leds[i], LOW);
  }
}

// =========================================================
// FUNCTION: procesarCadena
// PURPOSE: Validate and process an LED control command.
// FORMAT: Mxxxxxxxxxxx
// Each 'x' represents one LED: '1' = ON, '0' = OFF.
// =========================================================
void procesarCadena(char *cmd) {

  // Ignore commands that do not begin with 'M'.
  if (cmd[0] != 'M') {
    return;
  }

  // Update each LED according to its corresponding state.
  for (byte i = 0; i < 11; i++) {
    if (cmd[i + 1] == '1') {
      digitalWrite(leds[i], HIGH);
    } else {
      digitalWrite(leds[i], LOW);
    }
  }

  // Print the processed command to the USB serial monitor.
  Serial.print("Received command: ");
  Serial.println(cmd);
}

// =========================================================
// SETUP
// PURPOSE: Configure LED pins and serial interfaces.
// =========================================================
void setup() {

  // Configure all LED pins as outputs.
  for (byte i = 0; i < 11; i++) {
    pinMode(leds[i], OUTPUT);
  }

  // Start with every LED turned off.
  apagarTodos();

  // USB serial connection for debugging.
  Serial.begin(9600);

  // Serial connection to the PIC microcontroller.
  picSerial.begin(9600);

  Serial.println("Arduino ready");
  Serial.println("Waiting for command from PIC...");
}

// =========================================================
// MAIN LOOP
// PURPOSE: Read incoming serial data and process complete
//          commands when a newline character is received.
// =========================================================
void loop() {

  while (picSerial.available() > 0) {

    char c = picSerial.read();

    // A newline marks the end of a command.
    if (c == '\n') {

      // Terminate the buffer as a C string.
      buffer[indexBuffer] = '\0';

      // Process the completed command.
      procesarCadena(buffer);

      // Reset the buffer index for the next command.
      indexBuffer = 0;

    } else if (c != '\r') {

      // Store incoming characters while space is available.
      if (indexBuffer < sizeof(buffer) - 1) {
        buffer[indexBuffer] = c;
        indexBuffer++;

      } else {

        // Reset the buffer if the incoming command is too long.
        indexBuffer = 0;
      }
    }
  }
}
