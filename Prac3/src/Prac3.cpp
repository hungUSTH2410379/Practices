#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

void servo_timer0_init(void) {
  DDRD |= (1 << PD6);
  TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << CS02) | (1 << CS00);
  OCR0A = 23; // Vị trí giữa (~1.5 ms)
}

void led_pwm_timer1_init(void) {
  DDRB |= (1 << PB1);
  TCCR1A = (1 << COM1A1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A = 0; // Khởi đầu LED tắt
}

int main(void) {
  servo_timer0_init();
  led_pwm_timer1_init();

  while (1) {
    // Quét từ 0 đến 180 độ
    for (uint8_t pos = 0; pos <= 180; pos += 10) {
      OCR0A = 8 + ((uint32_t)pos * 33) / 180;
      OCR1A = (uint16_t)pos * 255 / 180;
      _delay_ms(10);
    }

    // Quét từ 180 về 0 độ
    for (int16_t pos = 180; pos >= 0; pos -= 10) {
      OCR0A = 16 + ((uint32_t)pos * 33) / 180;
      OCR1A = (uint16_t)pos * 255 / 180;
      _delay_ms(10);
    }
  }
}
