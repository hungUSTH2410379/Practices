#include <Arduino.h>
#include <LiquidCrystal.h>
#define SENSOR_PIN A0
const unsigned long SENSOR_INTERVAL = 1000; 
const unsigned long UART_INTERVAL = 1000;   

unsigned long prevSensorMillis = 0;
unsigned long prevUartMillis = 0;

int sensorValue = 0;
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Reading:");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - prevSensorMillis >= SENSOR_INTERVAL) {
        prevSensorMillis = currentMillis;
        sensorValue = analogRead(SENSOR_PIN);

        lcd.setCursor(0, 1);
        lcd.print(sensorValue);
        lcd.print("    "); 
    }

    if (currentMillis - prevUartMillis >= UART_INTERVAL) {
        prevUartMillis = currentMillis;

        Serial.print("Sensor Value: ");
        Serial.println(sensorValue);
    }
}