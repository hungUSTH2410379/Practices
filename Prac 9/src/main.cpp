#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SERVO_PIN PB1   // Arduino Pin 9 (OC1A)
#define MIN_ANGLE 0     // Minimum allowed angle
#define MAX_ANGLE 180   // Maximum allowed angle

/* =========================================================================
 *  USART Driver
 * ========================================================================= */

void usart_init(uint32_t baud) {
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);                  // Enable RX and TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);                // 8 data bits, 1 stop bit
}

bool usart_available(void) {
    return (UCSR0A & (1 << RXC0));
}

char usart_receive(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void usart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void usart_send_string(const char* str) {
    while (*str) usart_transmit(*str++);
}

// Parses integers from UART stream and flushes remaining characters
int usart_parse_int(void) {
    char buf[16];
    uint8_t idx = 0;
    
    while (1) {
        char c = usart_receive();
        if (c >= '0' && c <= '9') {
            if (idx < sizeof(buf) - 1) {
                buf[idx++] = c;
            }
        } else {
            // Non-digit encountered; flush remaining line characters from RX buffer
            _delay_ms(10);
            while (usart_available()) {
                usart_receive();
            }
            break;
        }
    }
    buf[idx] = '\0';
    return (idx > 0) ? atoi(buf) : -1;
}

/* =========================================================================
 *  Timer1 Servo Driver (50 Hz Fast PWM on OC1A / Pin 9)
 * ========================================================================= */

void servo_init(void) {
    DDRB |= (1 << SERVO_PIN); // Set PB1 (Pin 9) as output

    // Mode 14: Fast PWM with ICR1 as TOP
    // Non-inverting PWM on OC1A (PB1)
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler = 8

    // TOP value = (16MHz / (8 * 50Hz)) - 1 = 39999 -> 20ms period (50 Hz)
    ICR1 = 39999;
}

void servo_write(uint8_t angle) {
    if (angle > MAX_ANGLE) angle = MAX_ANGLE;
    
    // Map 0 - 180 degrees to 1088 ticks (544us) - 4800 ticks (2400us)
    uint16_t ticks = 1088 + ((uint32_t)angle * 3712) / 180;
    OCR1A = ticks;
}

/* =========================================================================
 *  Main Application
 * ========================================================================= */

int main(void) {
    usart_init(9600);
    servo_init();
    
    // Set initial position to 0 degrees
    servo_write(MIN_ANGLE);
    
    usart_send_string("=== ATmega328 Servo Control Ready ===\r\n");
    usart_send_string("Send an angle command (0 to 180):\r\n");

    char response[128];

    while (1) {
        if (usart_available()) {
            int targetAngle = usart_parse_int();

            if (targetAngle >= MIN_ANGLE && targetAngle <= MAX_ANGLE) {
                // Valid angle: rotate servo motor
                servo_write((uint8_t)targetAngle);
                
                snprintf(response, sizeof(response), 
                         "[CONFIRMATION] Command Accepted: Servo rotated to %d degrees.\r\n", 
                         targetAngle);
                usart_send_string(response);
                _delay_ms(1000);
            } else {
                // Invalid angle: return error message
                snprintf(response, sizeof(response), 
                         "[ERROR] Command Rejected: Angle %d is out of range! Permitted range is %d to %d degrees.\r\n", 
                         targetAngle, MIN_ANGLE, MAX_ANGLE);
                usart_send_string(response);
                _delay_ms(1000);
            }
        }
    }
}
