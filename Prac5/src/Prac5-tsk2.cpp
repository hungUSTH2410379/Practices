#include <Arduino.h>

const uint8_t PWM_PIN = 3;
volatile uint8_t dutyCycle = 0;

void setup() {
    pinMode(PWM_PIN, OUTPUT);

    cli();                  // Disable global interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    OCR1A = 4999;                        // 20 ms period at 16 MHz with 64 prescaler
    TCCR1B |= (1 << WGM12);              // CTC Mode
    TCCR1B |= (1 << CS11) | (1 << CS10) ; // Prescaler 64
    TIMSK1 |= (1 << OCIE1A);             // Enable Compare Match Interrupt
    sei();                  // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
    dutyCycle += 5;
    analogWrite(PWM_PIN, dutyCycle);
}

void loop() {
    // Foreground tasks run uninterrupted
}