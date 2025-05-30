// Constants & configuration
const unsigned long pollInterval = 1000;       // 1 sec polling
const unsigned long doorOpenThreshold = 30000; // 30 sec threshold
const unsigned long muteDuration = 15000;      // 15 sec mute duration

// Pins
//const int sensorPins[3] = {2, 3, 4};
const int sensorPins[3] = {5, 6, 7};  // 8 avail for future door
//const int buzzerPin = 9;
const int buzzerPin = 3; // PWM-capable
//const int muteButtonPin = 8;
const int muteButtonPin = 2; //INT-capable

// Pins: LEDs
//const int redPin = 5;
const int redPin = 9; // PWM-capable
//const int greenPin = 6;
const int greenPin = 10;  // PWM-capable
const int bluePin = 11; // PWM-capable

// Door state tracking
unsigned long doorOpenTime[3] = {0, 0, 0};
bool alarmTriggered[3] = {false, false, false};
unsigned long muteActivatedTime = 0;
bool muteActive = false;

void setup() {
  Serial.begin(9600);
  Serial.println("System Initialized.");

  // Setup sensor pins
  for (int i = 0; i < 3; i++)
    pinMode(sensorPins[i], INPUT_PULLUP);

  // Setup buzzer pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Setup RGB LED pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setLEDColor(255, 0, 255); // Initial Purple

  // Setup mute button
  pinMode(muteButtonPin, INPUT_PULLUP);
}

void loop() {
  static unsigned long lastPollTime = 0;
  static unsigned long lastBeepToggle = 0;
  static bool buzzerState = false;

  unsigned long currentTime = millis();

  // Check mute button
  if (digitalRead(muteButtonPin) == LOW) {
    if (!muteActive) {
      muteActive = true;
      muteActivatedTime = currentTime;
      Serial.println("[Mute activated]");
      for (int i = 0; i < 3; i++)
        alarmTriggered[i] = false;
    }
  }

  if (muteActive && currentTime - muteActivatedTime >= muteDuration) {
    muteActive = false;
    Serial.println("[Mute expired]");
  }

  if (currentTime - lastPollTime >= pollInterval) {
    lastPollTime = currentTime;

    bool anyDoorOpen = false;
    bool anyAlarmActive = false;

    for (int i = 0; i < 3; i++) {
      bool doorOpen = digitalRead(sensorPins[i]) == HIGH;

      if (doorOpen) {
        anyDoorOpen = true;

        if (doorOpenTime[i] == 0) {
          doorOpenTime[i] = currentTime;
          Serial.print("[Door ");
          Serial.print(i + 1);
          Serial.println(" OPEN detected]");
        }

        if (!muteActive && !alarmTriggered[i] && (currentTime - doorOpenTime[i] >= doorOpenThreshold)) {
          alarmTriggered[i] = true;
          Serial.print("[Door ");
          Serial.print(i + 1);
          Serial.println(" ALARM triggered]");
        }
      } else {
        if (doorOpenTime[i] != 0 || alarmTriggered[i]) {
          Serial.print("[Door ");
          Serial.print(i + 1);
          Serial.println(" CLOSED detected, resetting states]");
        }
        doorOpenTime[i] = 0;
        alarmTriggered[i] = false;
      }

      if (alarmTriggered[i])
        anyAlarmActive = true;
    }

    // LED state management
    if (anyAlarmActive)
      setLEDColor(255, 0, 0);       // Red
    else if (anyDoorOpen)
      setLEDColor(255, 60, 0);     // Orange
    else
      setLEDColor(0, 255, 0);       // Green
  }

  // Buzzer beep logic
  bool anyAlarmActive = alarmTriggered[0] || alarmTriggered[1] || alarmTriggered[2];

  if (anyAlarmActive) {
    if (currentTime - lastBeepToggle >= 250) {
      lastBeepToggle = currentTime;
      buzzerState = !buzzerState;

      if (buzzerState)
        // tone(buzzerPin, 2000);  // dynamic piezo
        analogWrite(buzzerPin, 255);  // fixed tone piezo (PWM duty changes tone)
      else
        // noTone(buzzerPin);  // dynamic piezo
        analogWrite(buzzerPin, 0);  // fixed tone piezo
    }
  } else {
    noTone(buzzerPin);
    buzzerState = false;
  }
}

// Helper: Set RGB LED color
void setLEDColor(byte red, byte green, byte blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
