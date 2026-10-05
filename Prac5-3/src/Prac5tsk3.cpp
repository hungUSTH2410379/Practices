#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

// Millis system tick using Timer0 CTC Mode
volatile uint32_t timer0_millis = 0;

void timer0_millis_init(void) {
    TCCR0A = (1 << WGM01); // CTC Mode
    OCR0A = 249;           // 16MHz / (64 * 1000) - 1 = 249 -> 1ms interrupt
    TIMSK0 = (1 << OCIE0A);
    TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
}

ISR(TIMER0_COMPA_vect) {
    timer0_millis++;
}

uint32_t millis(void) {
    uint32_t m;
    cli();
    m = timer0_millis;
    sei();
    return m;
}

// LCD Pins: RS=PB4 (D12), E=PB3 (D11), D4-D7=PD4-PD7 (D4-D7)
#define LCD_RS_PIN PB4
#define LCD_E_PIN  PB3

static void lcd_enable(void) {
    PORTB |= (1 << LCD_E_PIN);
    _delay_us(1);
    PORTB &= ~(1 << LCD_E_PIN);
    _delay_us(100);
}

static void lcd_command(uint8_t cmd) {
    PORTB &= ~(1 << LCD_RS_PIN);
    PORTD = (PORTD & 0x0F) | (cmd & 0xF0);
    lcd_enable();
    PORTD = (PORTD & 0x0F) | ((cmd << 4) & 0xF0);
    lcd_enable();
    if (cmd == 0x01 || cmd == 0x02) _delay_ms(2);
}

static void lcd_data(uint8_t data) {
    PORTB |= (1 << LCD_RS_PIN);
    PORTD = (PORTD & 0x0F) | (data & 0xF0);
    lcd_enable();
    PORTD = (PORTD & 0x0F) | ((data << 4) & 0xF0);
    lcd_enable();
}

void lcd_init(void) {
    DDRB |= (1 << LCD_RS_PIN) | (1 << LCD_E_PIN);
    DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    _delay_ms(50);

    PORTB &= ~(1 << LCD_RS_PIN);
    PORTD = (PORTD & 0x0F) | 0x30;
    lcd_enable(); _delay_ms(5);
    lcd_enable(); _delay_us(150);
    lcd_enable(); _delay_us(150);

    PORTD = (PORTD & 0x0F) | 0x20;
    lcd_enable(); _delay_us(150);

    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x01);
    lcd_command(0x06);
}

void lcd_print(const char* str) {
    while (*str) lcd_data((uint8_t)*str++);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr = (row == 0) ? col : (0x40 + col);
    lcd_command(0x80 | addr);
}

void ADC_init(void) {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

const uint32_t INTERVAL_MS = 100;
uint32_t previousMillis = 0;

int main(void) {
    timer0_millis_init();
    ADC_init();
    lcd_init();
    sei();

    lcd_set_cursor(0, 0);
    lcd_print("Sensor Reading:");

    char buf[17];

    while (1) {
        uint32_t currentMillis = millis();

        if (currentMillis - previousMillis >= INTERVAL_MS) {
            previousMillis = currentMillis;
            uint16_t sensorValue = ADC_read(0); // A0 = ADC Channel 0
            
            lcd_set_cursor(0, 1);
            snprintf(buf, sizeof(buf), "%-5u    ", sensorValue);
            lcd_print(buf); 
        }
    }
}