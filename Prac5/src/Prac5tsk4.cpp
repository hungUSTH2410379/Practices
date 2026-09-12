#include <Arduino.h>
#include <LiquidCrystal.h>

#define SENSOR_PIN A0

const unsigned long SENSOR_INTERVAL = 1000;  // 100 ms loop
const unsigned long UART_INTERVAL = 1000;   // 1000 ms loop

unsigned long prevSensorMillis = 0;
unsigned long prevUartMillis = 0;

int sensorValue = 0;

// LCD Pins: RS=PB4(12), E=PB3(11), D4=PD4(4), D5=PD5(5), D6=PD6(6), D7=PD7(7)
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

void setup() {
    Serial.begin(9600); // Initializes PD0 (RX) and PD1 (TX)
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Reading:");
}

void loop() {
    unsigned long currentMillis = millis();

    // Read sensor and refresh LCD every 100 ms
    if (currentMillis - prevSensorMillis >= SENSOR_INTERVAL) {
        prevSensorMillis = currentMillis;
        sensorValue = analogRead(SENSOR_PIN);

        lcd.setCursor(0, 1);
        lcd.print(sensorValue);
        lcd.print("    "); // Pad spaces to clear remaining characters
    }

    // Transmit reading over UART every 1000 ms
    if (currentMillis - prevUartMillis >= UART_INTERVAL) {
        prevUartMillis = currentMillis;

        Serial.print("Sensor Value: ");
        Serial.println(sensorValue);
    }
}