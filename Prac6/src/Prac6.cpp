#include <Arduino.h>

#define STEP_PIN   2
#define DIR_PIN    3
#define ENABLE_PIN 4

void stepMotor(uint16_t steps, bool dir, uint16_t step_delay_us) {
    digitalWrite(DIR_PIN, dir ? HIGH : LOW);
    
    for (uint16_t i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10); // A4988 needs minimum 1us HIGH pulse
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(step_delay_us);
    }
}

void setup() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    // Enable driver (Active LOW)
    digitalWrite(ENABLE_PIN, LOW);
}

void loop() {
    // 200 steps = 1 revolution (1.8° full step)
    stepMotor(2000, true, 4000);   // Clockwise
    delay(100);

   
}