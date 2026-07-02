#include <Servo.h>

#define PIR_PIN       7
#define REED_PIN      8

#define DOOR_LED      2
#define UV_STATUS_LED 3
#define BUZZER        4
#define MOTION_LED    5

#define SERVO_PIN     9
#define RELAY_PIN     10
#define UV_LED        11

Servo doorServo;

bool cycleRunning = false;

void setup() {

  pinMode(PIR_PIN, INPUT);
  pinMode(REED_PIN, INPUT_PULLUP);

  pinMode(DOOR_LED, OUTPUT);
  pinMode(UV_STATUS_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTION_LED, OUTPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(UV_LED, OUTPUT);

  doorServo.attach(SERVO_PIN);

  // Door starts OPEN
  doorServo.write(-90);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(UV_LED, LOW);
  digitalWrite(UV_STATUS_LED, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);
  Serial.println("Smart UV Sterilization System Ready");
}

void loop() {

  bool motion = digitalRead(PIR_PIN);

  // Door closed when magnet is near reed switch
  bool doorClosed = (digitalRead(REED_PIN) == LOW);

  // Indicator LEDs
  digitalWrite(MOTION_LED, motion);
  digitalWrite(DOOR_LED, doorClosed);

  // Start sterilization cycle only if safe
  if (doorClosed && !motion && !cycleRunning) {

    cycleRunning = true;

    Serial.println("Safe Conditions Met");

    // 3 second warning
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
      delay(500);
    }

    // Safety recheck
    motion = digitalRead(PIR_PIN);
    doorClosed = (digitalRead(REED_PIN) == LOW);

    if (doorClosed && !motion) {

      Serial.println("Closing Chamber");

      // Close door
      doorServo.write(90);
      delay(1000);

      Serial.println("UV Sterilization Started");

      digitalWrite(UV_STATUS_LED, HIGH);
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(UV_LED, HIGH);

      unsigned long startTime = millis();

      // Run UV for 15 seconds
      while (millis() - startTime < 15000) {

        motion = digitalRead(PIR_PIN);
        doorClosed = (digitalRead(REED_PIN) == LOW);

        digitalWrite(MOTION_LED, motion);
        digitalWrite(DOOR_LED, doorClosed);

        // Emergency shutdown
        if (motion || !doorClosed) {

          Serial.println("EMERGENCY SHUTDOWN");

          digitalWrite(RELAY_PIN, LOW);
          digitalWrite(UV_LED, LOW);
          digitalWrite(UV_STATUS_LED, LOW);

          // Open door immediately
          doorServo.write(-90);

          digitalWrite(BUZZER, HIGH);
          delay(2000);
          digitalWrite(BUZZER, LOW);

          break;
        }

        delay(100);
      }

      // Sterilization complete
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(UV_LED, LOW);
      digitalWrite(UV_STATUS_LED, LOW);

      Serial.println("Sterilization Complete");

      // Open door
      doorServo.write(-90);

      Serial.println("Cooldown Started");
      delay(5000);

      Serial.println("System Ready");
    }

    cycleRunning = false;
  }

  delay(100);
}