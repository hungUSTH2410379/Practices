#include <Arduino.h>
#include <LiquidCrystal.h>

#define SENSOR_PIN A0
const unsigned long INTERVAL_MS = 100;
unsigned long previousMillis = 0;

// Initialize LiquidCrystal(RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

void setup() {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Reading:");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= INTERVAL_MS) {
        previousMillis = currentMillis;

        int sensorValue = analogRead(SENSOR_PIN);

        lcd.setCursor(0, 1);
        lcd.print(sensorValue);
        lcd.print("    "); // Pad with spaces to clear old digits
    }
}