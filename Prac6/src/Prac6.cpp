#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define STEP_PIN   PD2
#define DIR_PIN    PD3
#define ENABLE_PIN PD4

void stepMotor(uint16_t steps, bool dir, uint16_t step_delay_us) {
    if (dir) {
        PORTD |= (1 << DIR_PIN);
    } else {
        PORTD &= ~(1 << DIR_PIN);
    }
    
    for (uint16_t i = 0; i < steps; i++) {
        PORTD |= (1 << STEP_PIN);
        _delay_us(10); // Minimum HIGH pulse
        PORTD &= ~(1 << STEP_PIN);
        
        // Microsecond variable delay loop
        for (uint16_t d = 0; d < step_delay_us; d++) {
            _delay_us(1);
        }
    }
}

int main(void) {
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN) | (1 << ENABLE_PIN);

    // Enable driver (Active LOW)
    PORTD &= ~(1 << ENABLE_PIN);

    while (1) {
        stepMotor(2000, true, 4000);   // Clockwise
        _delay_ms(100);
    }
}
