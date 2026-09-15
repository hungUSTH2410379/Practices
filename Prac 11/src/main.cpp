#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define BAUD 9600
#define BRC ((F_CPU / 16 / BAUD) - 1)
#define DARK_THRESHOLD 512  // LDR ADC threshold (adjust as needed)

// Common Anode Segment Encoding for 0-9 (1 = Segment ON in raw mask)
// Bit 0 = Segment A, Bit 1 = Segment B, ..., Bit 6 = Segment G
const uint8_t digit_map[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// Global array holding the 4 digits to display
volatile uint8_t display_buffer[4] = {0, 0, 0, 0};

// --- Function Prototypes ---
void init_hardware(void);
uint16_t adc_read(uint8_t channel);
void uart_print(const char* str);
void update_display_buffer(uint16_t value);

// --- Timer0 Interrupt: Multiplexes Common Anode 5641BS Display (~976 Hz) ---
ISR(TIMER0_OVF_vect) {
    static uint8_t current_digit = 0;
    
    // 1. Turn OFF all digits (Active LOW digit select: pull HIGH to disable)
    PORTC |= (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4);
    
    // 2. Invert segment bits for Common Anode (LOW = Segment ON, HIGH = Segment OFF)
    uint8_t mask = ~digit_map[display_buffer[current_digit]];
    
    // Output Segments A-F (Bits 0-5) to PD2-PD7
    PORTD = (PORTD & 0x03) | ((mask & 0x3F) << 2);
    
    // Output Segment G (Bit 6) to PB0
    PORTB = (PORTB & 0xFE) | ((mask & 0x40) >> 6);
    
    // 3. Turn ON the active digit (Active LOW digit select: pull LOW to enable)
    PORTC &= ~(1 << (PC1 + current_digit));
    
    // 4. Cycle to next digit
    current_digit++;
    if (current_digit > 3) {
        current_digit = 0;
    }
}

int main(void) {
    init_hardware();
    char serial_buffer[60];
    
    // Enable global interrupts for Timer0 multiplexing
    sei(); 
    
    uart_print("=== LDR Monitor (5641BS Common Anode) Ready ===\r\n");

    while (1) {
        // Read ADC from Channel 0 (A0 pin)
        uint16_t light_val = adc_read(0);
        
        // Update digit buffer for 7-segment rendering
        update_display_buffer(light_val);
        
        // Light condition decision: Low ADC = Dark, High ADC = Bright
        if (light_val < DARK_THRESHOLD) {
            PORTB |= (1 << PB5); // Turn ON LED on PB5 (Pin 13)
            sprintf(serial_buffer, "ADC: %04d | Condition: DARK   | LED: ON\r\n", light_val);
        } else {
            PORTB &= ~(1 << PB5); // Turn OFF LED on PB5 (Pin 13)
            sprintf(serial_buffer, "ADC: %04d | Condition: BRIGHT | LED: OFF\r\n", light_val);
        }
        
        // Transmit reading over UART
        uart_print(serial_buffer);
        
        // Non-blocking sampling interval (display multiplexing runs smoothly in ISR)
        _delay_ms(400); 
    }
    
    return 0;
}

void init_hardware(void) {
    // 1. Initialize UART (9600 Baud)
    UBRR0H = (BRC >> 8);
    UBRR0L = BRC;
    UCSR0B = (1 << TXEN0); 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // 2. Initialize ADC (Channel A0, AVCC Reference, 128 Prescaler)
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // 3. Initialize Timer0 for Display Multiplexing (Prescaler 64 -> ~976 Hz Overflow)
    TCCR0B = (1 << CS01) | (1 << CS00);
    TIMSK0 = (1 << TOIE0); 

    // 4. Configure GPIO Directions
    DDRD |= 0xFC;                                              // PD2-PD7 as Output (Segments A-F)
    DDRB |= (1 << PB0) | (1 << PB5);                           // PB0 Output (Seg G), PB5 Output (LED)
    DDRC |= (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4); // PC1-PC4 Output (Digits 1-4)
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC)); // Wait for conversion complete
    return ADC;
}

void update_display_buffer(uint16_t value) {
    display_buffer[0] = (value / 1000) % 10; // Thousands
    display_buffer[1] = (value / 100) % 10;  // Hundreds
    display_buffer[2] = (value / 10) % 10;   // Tens
    display_buffer[3] = value % 10;          // Ones
}

void uart_print(const char* str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0))); 
        UDR0 = *str++;                    
    }
}