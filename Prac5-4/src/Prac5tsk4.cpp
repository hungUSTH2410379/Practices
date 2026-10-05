#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

volatile uint32_t timer0_millis = 0;

void timer0_millis_init(void) {
    TCCR0A = (1 << WGM01);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
    TCCR0B = (1 << CS01) | (1 << CS00);
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

void USART_init(uint32_t baud) {
    uint16_t ubrr = (F_CPU / 16 / baud) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void USART_send_string(const char* str) {
    while (*str) USART_transmit(*str++);
}

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

const uint32_t SENSOR_INTERVAL = 1000; 
const uint32_t UART_INTERVAL   = 1000;   

uint32_t prevSensorMillis = 0;
uint32_t prevUartMillis   = 0;

uint16_t sensorValue = 0;

int main(void) {
    timer0_millis_init();
    USART_init(9600);
    ADC_init();
    lcd_init();
    sei();

    lcd_set_cursor(0, 0);
    lcd_print("Sensor Reading:");

    char buf[17];

    while (1) {
        uint32_t currentMillis = millis();

        if (currentMillis - prevSensorMillis >= SENSOR_INTERVAL) {
            prevSensorMillis = currentMillis;
            sensorValue = ADC_read(0);

            lcd_set_cursor(0, 1);
            snprintf(buf, sizeof(buf), "%-5u    ", sensorValue);
            lcd_print(buf); 
        }

        if (currentMillis - prevUartMillis >= UART_INTERVAL) {
            prevUartMillis = currentMillis;

            snprintf(buf, sizeof(buf), "Sensor Value: %u\r\n", sensorValue);
            USART_send_string(buf);
        }
    }
}