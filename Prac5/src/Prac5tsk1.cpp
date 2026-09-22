void ADC_init() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
volatile uint16_t rawLight = 0;

void Task_ReadADC() {
    rawLight = ADC_read(0);

    lcd.setCursor(0, 0);
    lcd.print("Light: ");
    lcd.print(rawLight);
    lcd.print("   ");
}