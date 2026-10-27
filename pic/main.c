
/*
 * Project: Real-Time Computer Vision Rehabilitation System
 *
 * Purpose:
 *   Receives posture assessment data from the Python application
 *   through USB CDC, displays feedback on a 16x2 LCD, and sends
 *   LED control commands to an Arduino through UART.
 *
 * Microcontroller: PIC18F45K50
 *
 * Communication:
 *   - USB CDC: Python application -> PIC
 *   - UART: PIC -> Arduino LED controller
 *
 * Display: 16x2 character LCD
 *
 * Note:
 *   Educational engineering prototype; not intended for
 *   clinical diagnosis or as a certified medical device.
 */

#include "system.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "usb.h"
#include "app_led_usb_status.h"
#include "app_device_cdc_basic.h"
#include "usb_config.h"

// =========================================================
// HARDWARE CONFIGURATION
// =========================================================

#define _XTAL_FREQ 48000000UL

// LCD control pins.
#define RS LATDbits.LATD5
#define E  LATDbits.LATD4

// USB CDC receive buffer.
static uint8_t readBuffer[CDC_DATA_OUT_EP_SIZE];

// Indicates whether the UART peripheral has been initialized.
static uint8_t uart_iniciado = 0;


// =========================================================
// UART COMMUNICATION
// PURPOSE:
//   Configure the PIC UART interface and transmit LED control
//   data to the Arduino.
// CONFIGURATION:
//   Baud rate: 9600 bps
//   Oscillator frequency: 48 MHz
// =========================================================

void UART_Init(void) {

    // Configure UART transmit and receive pins.
    TRISCbits.TRISC6 = 0;   // TX output
    TRISCbits.TRISC7 = 1;   // RX input

    // Configure asynchronous high-speed UART mode.
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;

    // Baud-rate generator configuration.
    // SPBRG = 0x04E1 (1249), as used in the original project.
    SPBRGH = 0x04;
    SPBRG  = 0xE1;

    // Enable the serial port and transmitter/receiver.
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1;
}


// Transmit one character when the UART transmit register is ready.
void UART_SendChar(char dato) {

    while (PIR1bits.TXIF == 0);

    TXREG = dato;
}


// Send a specified number of '0' characters.
// Each character represents an LED in the OFF state.
void UART_SendZeros(unsigned char cantidad) {

    unsigned char i;

    for (i = 0; i < cantidad; i++) {
        UART_SendChar('0');
    }
}


// Convert a logical condition into an ASCII LED state.
// '1' = LED ON; '0' = LED OFF.
void UART_SendBit(char condicion) {

    if (condicion) {
        UART_SendChar('1');
    } else {
        UART_SendChar('0');
    }
}


// =========================================================
// LED FEEDBACK MAPPING
// PURPOSE:
//   Extract selected posture fields from the received USB CDC
//   data and generate the LED control command for Arduino.
//
// OUTPUT FORMAT:
//   'M' + 11 LED state characters + newline
//
// NOTE:
//   The input indices below are preserved from the original
//   implementation and depend on the expected input format.
// =========================================================

void UART_SendLedInfo(void) {

    char p;
    char b;
    char r;

    // Extract selected fields from the received data.
    r = readBuffer[1];
    p = readBuffer[4];
    b = readBuffer[7];

    // Start the Arduino LED control command.
    UART_SendChar('M');

    // If the selected conditions indicate an invalid state,
    // turn off all LEDs.
    if ((r == '0' || p == '0') && b == '0') {

        UART_SendZeros(11);

    } else if (p == '1') {

        // Position 1: activate the corresponding LED pattern.
        UART_SendBit(r == '1'); // LED 1 in the transmitted pattern
        UART_SendBit(r == '1'); // LED 2 in the transmitted pattern
        UART_SendBit(b == '1'); // LED 3 in the transmitted pattern
        UART_SendZeros(8);      // Remaining 8 LEDs OFF

    } else if (p == '2') {

        // Position 2: activate the corresponding LED pattern.
        UART_SendZeros(3);      // First 3 LEDs OFF
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(b == '1'); // Next LED
        UART_SendZeros(4);      // Remaining 4 LEDs OFF

    } else if (p == '3') {

        // Position 3: activate the corresponding LED pattern.
        UART_SendZeros(7);      // First 7 LEDs OFF
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(r == '1'); // Next LED
        UART_SendBit(b == '1'); // Final LED
    }

    // Mark the end of the command for the Arduino receiver.
    UART_SendChar('\n');
}


// =========================================================
// LCD DISPLAY FUNCTIONS
// PURPOSE:
//   Control a 16x2 character LCD using a 4-bit data interface.
// =========================================================

// Send a 4-bit data nibble to the LCD.
void ENVIA_NIBBLE(unsigned char dato_n) {

    LATD = (LATD & 0xF0) | ((dato_n >> 4) & 0x0F);

    E = 1;
    __delay_us(40);
    E = 0;
}


// Send a command instruction to the LCD.
void ENVIA_LCD_CMD(unsigned char cmd) {

    RS = 0;

    ENVIA_NIBBLE(cmd);
    ENVIA_NIBBLE(cmd << 4);

    __delay_ms(2);
}


// Send a character to the LCD.
void ENVIA_CHAR(unsigned char dato_c) {

    RS = 1;

    ENVIA_NIBBLE(dato_c);
    ENVIA_NIBBLE(dato_c << 4);

    __delay_us(50);
}


// Clear the LCD display.
void BORRAR_LCD(void) {

    ENVIA_LCD_CMD(0x01);
    __delay_ms(2);
}


// Set the LCD cursor position.
// Rows are numbered 1 and 2; columns start at 0.
void POS_CURSOR(unsigned char fila, unsigned char columna) {

    if (fila == 1) {
        ENVIA_LCD_CMD(0x80 + columna);
    } else {
        ENVIA_LCD_CMD(0xC0 + columna);
    }
}


// Write a string containing the specified number of characters.
void ESCRIBE_MENSAJE(const char *cadena, unsigned char tam) {

    unsigned char i;

    for (i = 0; i < tam; i++) {
        ENVIA_CHAR(cadena[i]);
    }
}


// Initialize the LCD in 4-bit mode.
void LCD_CONFIG(void) {

    // Configure LCD data and control pins as outputs.
    TRISD = 0x00;
    LATD = 0x00;

    RS = 0;
    E = 0;

    // Wait for the LCD power-up sequence.
    __delay_ms(20);

    // Initialize the LCD interface.
    ENVIA_NIBBLE(0x30);
    __delay_ms(5);

    ENVIA_NIBBLE(0x30);
    __delay_us(200);

    ENVIA_NIBBLE(0x30);
    ENVIA_NIBBLE(0x20);

    // Configure 4-bit interface, display, cursor movement,
    // and clear the display.
    ENVIA_LCD_CMD(0x28);
    ENVIA_LCD_CMD(0x0C);
    ENVIA_LCD_CMD(0x06);
    ENVIA_LCD_CMD(0x01);

    __delay_ms(2);
}


// =========================================================
// USB CDC INITIALIZATION
// PURPOSE:
//   Configure USB serial parameters and display the startup
//   status message on the LCD.
// =========================================================

void APP_DeviceCDCBasicDemoInitialize(void) {

    // Configure USB CDC communication parameters.
    line_coding.bCharFormat = 0;
    line_coding.bDataBits = 8;
    line_coding.bParityType = 0;
    line_coding.dwDTERate = 9600;

    // Initialize the LCD and display the startup status.
    LCD_CONFIG();
    BORRAR_LCD();

    POS_CURSOR(1, 0);
    ESCRIBE_MENSAJE("USB CDC LISTO", 13);

    POS_CURSOR(2, 0);
    ESCRIBE_MENSAJE("Esperando...", 12);
}


// =========================================================
// MAIN USB CDC TASK
// PURPOSE:
//   1. Verify USB connection state.
//   2. Initialize UART when needed.
//   3. Read data received from the Python application.
//   4. Display posture feedback on the LCD.
//   5. Generate and transmit LED states to Arduino.
//   6. Service USB CDC communication.
// =========================================================

void APP_DeviceCDCBasicDemoTasks(void) {

    uint8_t numBytesRead;
    char r;
    char p;

    // Process application data only when USB is configured.
    if (USBGetDeviceState() < CONFIGURED_STATE) {
        return;
    }

    // Do not process data while the USB device is suspended.
    if (USBIsDeviceSuspended() == true) {
        return;
    }

    // Initialize UART once before transmitting LED commands.
    if (uart_iniciado == 0) {
        UART_Init();
        uart_iniciado = 1;
    }

    // Read available bytes from the USB CDC receive endpoint.
    numBytesRead = getsUSBUSART(
        readBuffer,
        sizeof(readBuffer) - 1
    );

    if (numBytesRead > 0) {

        // Add a string terminator to the received data.
        readBuffer[numBytesRead] = '\0';

        // Extract the fields used by the original LCD logic.
        r = readBuffer[1];
        p = readBuffer[4];

        BORRAR_LCD();

        // -------------------------------------------------
        // LCD ROW 1: SPINE / POSTURE FEEDBACK
        // -------------------------------------------------
        POS_CURSOR(1, 0);

        if (r == '0') {

            ENVIA_CHAR('E');
            ENVIA_CHAR('N');
            ENVIA_CHAR('D');
            ENVIA_CHAR('E');
            ENVIA_CHAR('R');
            ENVIA_CHAR('E');
            ENVIA_CHAR('Z');
            ENVIA_CHAR('A');
            ENVIA_CHAR(' ');
            ENVIA_CHAR('C');
            ENVIA_CHAR('O');
            ENVIA_CHAR('L');
            ENVIA_CHAR('U');
            ENVIA_CHAR('M');
            ENVIA_CHAR('N');
            ENVIA_CHAR('A');

        } else if (r == '1') {

            ENVIA_CHAR('P');
            ENVIA_CHAR('O');
            ENVIA_CHAR('S');
            ENVIA_CHAR('T');
            ENVIA_CHAR('U');
            ENVIA_CHAR('R');
            ENVIA_CHAR('A');
            ENVIA_CHAR(' ');
            ENVIA_CHAR('C');
            ENVIA_CHAR('O');
            ENVIA_CHAR('R');
            ENVIA_CHAR('R');
            ENVIA_CHAR('E');
            ENVIA_CHAR('C');
            ENVIA_CHAR('T');
            ENVIA_CHAR('A');
        }

        // -------------------------------------------------
        // LCD ROW 2: ARM POSITION FEEDBACK
        // -------------------------------------------------
        POS_CURSOR(2, 0);

        if (p == '0') {

            ENVIA_CHAR('E');
            ENVIA_CHAR('N');
            ENVIA_CHAR('D');
            ENVIA_CHAR('E');
            ENVIA_CHAR('R');
            ENVIA_CHAR('E');
            ENVIA_CHAR('Z');
            ENVIA_CHAR('A');
            ENVIA_CHAR(' ');
            ENVIA_CHAR('B');
            ENVIA_CHAR('R');
            ENVIA_CHAR('A');
            ENVIA_CHAR('Z');
            ENVIA_CHAR('O');
            ENVIA_CHAR(' ');
            ENVIA_CHAR(' ');

        } else if (p == '1' || p == '2' || p == '3') {

            ENVIA_CHAR('B');
            ENVIA_CHAR('R');
            ENVIA_CHAR('A');
            ENVIA_CHAR('Z');
            ENVIA_CHAR('O');
            ENVIA_CHAR(' ');
            ENVIA_CHAR('R');
            ENVIA_CHAR('E');
            ENVIA_CHAR('C');
            ENVIA_CHAR('T');
            ENVIA_CHAR('O');
            ENVIA_CHAR(' ');
            ENVIA_CHAR('P');
            ENVIA_CHAR(p);
            ENVIA_CHAR(' ');
            ENVIA_CHAR(' ');
        }

        // Generate the LED pattern and send it to Arduino.
        UART_SendLedInfo();

        // Echo the received data to the USB host when ready.
        if (USBUSARTIsTxTrfReady()) {
            putUSBUSART(readBuffer, numBytesRead);
        }

        // Clear the receive buffer before the next read.
        memset(readBuffer, 0, sizeof(readBuffer));
    }

    // Service pending USB CDC transmissions.
    CDCTxService();
}
