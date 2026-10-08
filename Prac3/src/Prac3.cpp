#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

void servo_timer0_init(void) {
  DDRB |= (1 << PB1);
  TCCR1A = (1 << COM1A1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A = 23; 
}

int main(void) {
  servo_timer0_init();
  
  while (1) {
    // Quét từ 0 đến 180 độ
    for (uint8_t pos = 0; pos <= 180; pos += 10) {
      OCR1A = 8 + ((uint32_t)pos * 33) / 180;
      _delay_ms(10);
    }

    // Quét từ 180 về 0 độ
    for (int16_t pos = 180; pos >= 0; pos -= 10) {
      OCR1A = 16 + ((uint32_t)pos * 33) / 180;
      _delay_ms(10);
    }
  }
}
