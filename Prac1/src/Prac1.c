#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRC |= (1 << PC0);      
    PORTC &= ~(1 << PC0);   

    DDRD &= ~(1 << PD2);     
    PORTD |= (1 << PD2);     

    uint8_t lastButtonState = 1; 

    while (1) {
        uint8_t currentButtonState = (PIND & (1 << PD2)) >> PD2;

        if (lastButtonState == 1 && currentButtonState == 0) {
            PORTC ^= (1 << PC0);
            _delay_ms(50);
        }
        lastButtonState = currentButtonState;
    }
    return 0;
}