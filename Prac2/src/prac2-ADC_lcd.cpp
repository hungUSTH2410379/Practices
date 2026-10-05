#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

// LCD Pin Definitions (RS=PB0/D8, E=PB1/D9, D4-D7=PD4-PD7/D4-D7)
#define LCD_RS_PIN PB0
#define LCD_E_PIN  PB1

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

    PORTD = (PORTD & 0x0F) | 0x20; // Switch to 4-bit mode
    lcd_enable(); _delay_us(150);

    lcd_command(0x28); // 4-bit mode, 2 lines, 5x8 font
    lcd_command(0x0C); // Display ON, cursor OFF
    lcd_command(0x01); // Clear display
    lcd_command(0x06); // Entry mode
}

void lcd_print(const char* str) {
    while (*str) lcd_data((uint8_t)*str++);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr = (row == 0) ? col : (0x40 + col);
    lcd_command(0x80 | addr);
}

void lcd_clear(void) {
    lcd_command(0x01);
}

volatile uint16_t adc_sum = 0;
volatile uint8_t sample_count = 0;
volatile bool adc_ready = false;

void ADC_Init(void) {
    ADMUX |= (1 << REFS0) | (1 << MUX1);
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

ISR(ADC_vect) {
    adc_sum += ADC;
    sample_count++;
    
    if (sample_count >= 16) {
        adc_ready = true;
    } else {
        ADCSRA |= (1 << ADSC);
    }
}

int main(void) {
    lcd_init();      
    lcd_print("System Ready...");
    _delay_ms(1000);
    lcd_clear();
    
    ADC_Init(); 
    sei();

    char buffer[17];

    while (1) {
        if (!adc_ready && sample_count == 0) {
            ADCSRA |= (1 << ADSC); 
        }

        if (adc_ready) {
            uint16_t avg_adc = adc_sum / 16;
            float voltage = (avg_adc * 5.0f) / 1024.0f;
            
            lcd_set_cursor(0, 0); 
            snprintf(buffer, sizeof(buffer), "Raw ADC: %-5u", avg_adc);
            lcd_print(buffer);
            
            lcd_set_cursor(0, 1);
            char volt_str[10];
            dtostrf(voltage, 4, 2, volt_str);
            snprintf(buffer, sizeof(buffer), "Voltage: %sV ", volt_str);
            lcd_print(buffer);
            
            adc_sum = 0;
            sample_count = 0;
            adc_ready = false;
            
            _delay_ms(10);
        }
    }
}
