#include <Arduino.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile uint16_t adc_sum = 0;
volatile uint8_t sample_count = 0;
volatile bool adc_ready = false;

void ADC_Init() {
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

void setup() {
    lcd.begin(16, 2);      
    lcd.print("System Ready...");
    delay(1000);
    lcd.clear();
    
    ADC_Init(); 
}

void loop() {
    if (!adc_ready && sample_count == 0) {
        ADCSRA |= (1 << ADSC); 
    }

    if (adc_ready) {
        uint16_t avg_adc = adc_sum / 16;
        float voltage = (avg_adc * 5.0) / 1024.0;
        
        lcd.setCursor(0, 0); 
        lcd.print("Raw ADC: ");
        lcd.print(avg_adc);
        lcd.print("   ");
        
        lcd.setCursor(0, 1); 
        lcd.print("Voltage: ");
        lcd.print(voltage);
        lcd.print("V  ");
        
        adc_sum = 0;
        sample_count = 0;
        adc_ready = false;
        
        delay(10);
    }
}