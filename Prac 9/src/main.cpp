#include <Arduino.h>
#include <Servo.h>
#define SERVO_PIN 9      // Digital pin connected to the Servo control line
#define MIN_ANGLE 0      // Minimum allowed angle
#define MAX_ANGLE 180    // Maximum allowed angle

Servo myServo;
void setup() {
    // Initialize serial communication at 9600 baud rate
    Serial.begin(9600);
    // Attach the servo motor to PWM pin 9
    myServo.attach(SERVO_PIN);
    // Set initial position to 0 degrees
    myServo.write(MIN_ANGLE);
    Serial.println("=== ATmega328 Servo Control Ready ===");
    Serial.println("Send an angle command (0 to 180):");
}
void loop() {
    // Check if data is available on the Serial buffer
    if (Serial.available() > 0) {
        // Read input as integer
        int targetAngle = Serial.parseInt();
        // Clear remaining buffer characters (e.g., \n or \r)
        while (Serial.available() > 0) {
            Serial.read();
        }
        // Validate angle range
        if (targetAngle >= MIN_ANGLE && targetAngle <= MAX_ANGLE) {
            // Valid angle: rotate servo motor
            myServo.write(targetAngle);
            // Send success confirmation message
            Serial.print("[CONFIRMATION] Command Accepted: Servo rotated to ");
            Serial.print(targetAngle);
            Serial.println(" degrees.");
            delay(1000);
        } else {
            // Invalid angle: return error message
            Serial.print("[ERROR] Command Rejected: Angle ");
            Serial.print(targetAngle);
            Serial.print(" is out of range! Permitted range is ");
            Serial.print(MIN_ANGLE);
            Serial.print(" to ");
            Serial.print(MAX_ANGLE);
            Serial.println(" degrees.");
            delay(1000);
        }
    }
}