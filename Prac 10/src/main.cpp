#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define PIR_PIN PD2     // Digital input pin connected to PIR Sensor OUT (Arduino Pin 2)
#define LED_PIN PB5     // Digital output pin for LED (Arduino Pin 13)

/* =========================================================================
 *  USART Driver
 * ========================================================================= */

void usart_init(uint32_t baud) {
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);                             // Enable Transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);            // 8 data bits, 1 stop bit
}

void usart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void usart_send_string(const char* str) {
    while (*str) usart_transmit(*str++);
}

/* =========================================================================
 *  Main Application
 * ========================================================================= */

int main(void) {
    usart_init(9600);

    // Configure PIR_PIN (PD2) as input and LED_PIN (PB5) as output
    DDRD &= ~(1 << PIR_PIN);
    DDRB |= (1 << LED_PIN);

    // Set LED initial state to LOW
    PORTB &= ~(1 << LED_PIN);

    usart_send_string("=== ATmega328 PIR Motion Detector Ready ===\r\n");

    int8_t lastPirState = -1;  // Initialize to -1 to trigger state check on startup

    while (1) {
        // Read input state from PIND register
        int8_t currentPirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;

        // Only process and transmit when the sensor state changes
        if (currentPirState != lastPirState) {
            if (currentPirState == 1) {
                PORTB |= (1 << LED_PIN);  // Turn LED ON
                usart_send_string("[STATUS] Motion Detected!\r\n");
            } else {
                PORTB &= ~(1 << LED_PIN); // Turn LED OFF
                usart_send_string("[STATUS] No Motion Detected.\r\n");
            }

            // Save state to prevent repeated duplicate serial messages
            lastPirState = currentPirState;
        }

        _delay_ms(50); // Small delay for reading stability
    }
}
