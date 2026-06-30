// ---------------- PIN DEFINITIONS ----------------
const int PIR_PIN = 2;
const int DOOR_PIN = 3;
const int BUTTON_PIN = 4;
const int BUZZER_PIN = 8;
const int RELAY_PIN = 10;
const int RED_LED = 11;
const int GREEN_LED = 12;

// ---------------- SYSTEM STATES ----------------
enum State { IDLE, RUNNING, ALARM };
State currentState = IDLE;

// ---------------- TIMING ----------------
unsigned long startTime = 0;
const unsigned long sterilizeDuration = 10000; // 10 sec

// Debounce
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;

// PIR stability
int pirStableCount = 0;
const int pirThreshold = 5;

// ---------------- SETUP ----------------
void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  Serial.begin(9600);

  digitalWrite(RELAY_PIN, HIGH); // UV OFF
  digitalWrite(RED_LED, HIGH);   // Idle
  digitalWrite(GREEN_LED, LOW);
}

// ---------------- MAIN LOOP ----------------
void loop() {
  handleButton();
  handleSystem();
}

// ---------------- BUTTON HANDLING ----------------
void handleButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {

    if (millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();

      Serial.println("Button Pressed");

      if (currentState == IDLE) {

        if (isSafe()) {
          Serial.println("SAFE → Starting Sterilization");
          startSterilization();
        } else {
          Serial.println("UNSAFE → Alarm Triggered");
          triggerAlarm(500);
        }
      }
    }
  }
}

// ---------------- SAFETY CHECK ----------------
bool isSafe() {
  return (digitalRead(DOOR_PIN) == LOW && isPIRSafe());
}

// PIR noise filtering
bool isPIRSafe() {
  if (digitalRead(PIR_PIN) == LOW) {
    if (pirStableCount < pirThreshold) pirStableCount++;
  } else {
    pirStableCount = 0;
  }

  return pirStableCount >= pirThreshold;
}

// ---------------- START PROCESS ----------------
void startSterilization() {
  currentState = RUNNING;

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RELAY_PIN, LOW); // UV ON

  startTime = millis();
}

// ---------------- SYSTEM HANDLER ----------------
void handleSystem() {

  if (currentState == RUNNING) {

    // Emergency condition (filtered PIR)
    if (digitalRead(DOOR_PIN) == HIGH || !isPIRSafe()) {
      Serial.println("EMERGENCY STOP!");
      emergencyStop();
      return;
    }

    // Timer complete
    if (millis() - startTime >= sterilizeDuration) {
      Serial.println("Completed Successfully");
      stopSterilization();
    }
  }
}

// ---------------- STOP NORMAL ----------------
void stopSterilization() {
  digitalWrite(RELAY_PIN, HIGH); // UV OFF
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  currentState = IDLE;
}

// ---------------- EMERGENCY STOP ----------------
void emergencyStop() {
  digitalWrite(RELAY_PIN, HIGH); // OFF immediately
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  currentState = ALARM;

  triggerAlarm(2000);

  currentState = IDLE;
}

// ---------------- BUZZER ----------------
void triggerAlarm(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);  // short blocking OK
  digitalWrite(BUZZER_PIN, LOW);
}