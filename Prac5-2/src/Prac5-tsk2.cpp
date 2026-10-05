#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t dutyCycle = 0;

void timer2_pwm_init(void) {
    DDRD |= (1 << PD3); // PD3 = Arduino Pin 3 (OC2B)
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); // Fast PWM Mode
    TCCR2B = (1 << CS22); // Prescaler 64 (~490 Hz PWM)
    OCR2B = 0;
}

int main(void) {
    timer2_pwm_init();

    cli();                
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    OCR1A = 4999;                        
    TCCR1B |= (1 << WGM12);          
    TCCR1B |= (1 << CS11) | (1 << CS10); 
    TIMSK1 |= (1 << OCIE1A);            
    sei();                

    while (1) {
    }
}

ISR(TIMER1_COMPA_vect) {
    dutyCycle += 5;
    OCR2B = dutyCycle; // Hardware PWM assignment on pin PD3
}