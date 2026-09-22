#include <Arduino.h>

const uint8_t PWM_PIN = 3;
volatile uint8_t dutyCycle = 0;
void setup() {
    pinMode(PWM_PIN, OUTPUT);

    cli();                
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    OCR1A = 4999;                        
    TCCR1B |= (1 << WGM12);          
    TCCR1B |= (1 << CS11) | (1 << CS10) ; 
    TIMSK1 |= (1 << OCIE1A);            
    sei();                
}

ISR(TIMER1_COMPA_vect) {
    dutyCycle += 5;
    analogWrite(PWM_PIN, dutyCycle);
}

void loop() {
}