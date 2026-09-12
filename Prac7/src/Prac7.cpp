#ifndef F_CPU
#define F_CPU 16000000UL   // only define if the build system hasn't already
#endif
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
 
#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)
 
// ==================== USART (Serial) ====================
void usart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    UCSR0B = (1 << TXEN0);                 // enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 data bits, 1 stop bit, no parity
}
 
void usart_transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));  // wait for empty transmit buffer
    UDR0 = data;
}
 
void usart_print(const char *str) {
    while (*str) usart_transmit(*str++);
}
 
void usart_print_ulong(uint32_t value) {
    char buf[11];
    ultoa(value, buf, 10);
    usart_print(buf);
}
 
void usart_print_float(float value, uint8_t decimals) {
    char buf[16];
    dtostrf(value, 0, decimals, buf);
    usart_print(buf);
}
 
// ---- shared between ISR and main loop ----
volatile uint16_t last_icr        = 0;
volatile uint32_t last_ovf        = 0;
volatile uint32_t ovf_count       = 0;
volatile uint32_t period_ticks    = 0;
volatile uint8_t  new_period_flag = 0;
 
// ==================== Timer1: Input Capture ====================
void timer1_capture_init(void) {
    DDRB  &= ~(1 << PB0);   // ICP1 = PB0 as input
    PORTB |=  (1 << PB0);   // internal pull-up
 
    TCCR1A = 0;
    TCCR1B = (1 << ICNC1)   // input noise canceler ON
           | (1 << ICES1)   // capture on RISING edge
           | (1 << CS11);   // prescaler = 8 -> timer tick = 0.5 us @16MHz
 
    TIMSK1 = (1 << ICIE1) | (1 << TOIE1); // enable capture + overflow interrupts
    TCNT1  = 0;
}
 
// Timer1 overflow -> extend capture range for slow signals
ISR(TIMER1_OVF_vect) {
    ovf_count++;
}
 
// Timer1 input capture -> a rising edge just happened on ICP1
ISR(TIMER1_CAPT_vect) {
    uint16_t icr_now = ICR1;
    uint32_t ovf_now = ovf_count;
 
    // ticks elapsed since the previous rising edge (overflow-safe)
    period_ticks = ((ovf_now - last_ovf) << 16) + (uint16_t)(icr_now - last_icr);
 
    last_icr = icr_now;
    last_ovf = ovf_now;
    new_period_flag = 1;
}
 
int main(void) {
    usart_init();
    timer1_capture_init();
    sei();  // enable global interrupts
 
    usart_print("Practice 7: Input Capture - Timer1 - ICP1\r\n");
    usart_print("Waiting for a signal on D8 (ICP1)...\r\n");
 
    while (1) {
        if (new_period_flag) {
            new_period_flag = 0;
 
            // atomic read of the shared 32-bit value
            cli();
            uint32_t ticks = period_ticks;
            sei();
 
            if (ticks > 0) {
                float    period_ms = ticks / 2000.0f;    // ticks * 0.5us, in ms
                uint32_t freq_hz   = 2000000UL / ticks;  // Timer1 clock = F_CPU/8
 
                usart_print("Period: ");
                usart_print_float(period_ms, 3);
                usart_print(" ms\tFrequency: ");
                usart_print_ulong(freq_hz);
                usart_print(" Hz\r\n");
            }
        }
    }
}
 