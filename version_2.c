#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <TimeLib.h>

// === Pin Mapping ===
#define TRIG_PIN A4
#define ECHO_PIN A5
#define BUZZER_PIN 10
#define SERVO_PIN 11

// === Keypad Setup ===
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {{'1', '2', '3', 'A'},
                         {'4', '5', '6', 'B'},
                         {'7', '8', '9', 'C'},
                         {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === LCD Setup ===
LiquidCrystal lcd(12, 13, A0, A1, A2, A3);

// === State ===
Servo lockServo;
int mode = 0;
time_t alarmTime;
int alarmLockDuration = 0;
time_t lockEndTime;
bool isLocked = false;
bool alarmSet = false;
bool lockSet = false;
bool alarmTriggered = false;
bool cancelled = false;
unsigned long lastToggleTime = 0;
bool showAlarmLine = false;
bool buzzerOn = false;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lockServo.attach(SERVO_PIN);
  lockServo.write(0);

  lcd.begin(16, 2);
  lcd.print("Set Time (HHMM):");
  setTime(getTimeFromUser("Set Time"));
  lcd.clear();
  lcd.print("Press A (alarm)");
  lcd.setCursor(0, 1);
  lcd.print("or B (lock)");
  delay(2500);
}

void loop() {
  char key = keypad.getKey();
  if (key == 'A') {
    handleAlarmSetup();
  } else if (key == 'B') {
    handleLockOnly();
  } else if (key == 'C') {
    if (alarmSet) {
      alarmSet = false;
      lcd.clear();
      lcd.print("Alarm cleared");
    } else {
      lcd.clear();
      lcd.print("No alarm active");
    }
    delay(1500);
  }

  displayStatus();

  if (alarmSet && now() >= alarmTime && !alarmTriggered) {
    alarmTriggered = true;
    lcd.clear();
    lcd.print("ALARM! Place");
    lcd.setCursor(0, 1);
    lcd.print("phone in box");
    delay(1000);
    handleAlarmTriggered();
  }

  if (lockSet && now() >= lockEndTime && isLocked) {
    unlockBox();
    lockSet = false;
  }

  delay(300);
}

// === Alarm Handler ===
void handleAlarmSetup() {
  if (alarmSet) {
    if (!confirmAlarmOverride(alarmTime)) return;
  }

  alarmTime = getTimeFromUser("Alarm");
  if (cancelled) return;

  alarmLockDuration = getDurationFromUser("Alarm lock (MM)");
  if (cancelled) return;

  alarmSet = true;
  lcd.clear();
  lcd.print("Alarm set for");
  lcd.setCursor(0, 1);
  print2Digits(hour(alarmTime));
  lcd.print(":");
  print2Digits(minute(alarmTime));
  delay(2000);
}

// === Lock Mode ===
void handleLockOnly() {
  if (lockSet) {
    lcd.clear();
    lcd.print("Lock in progress");
    delay(2000);
    return;
  }

  int minutes = getDurationFromUser("Lock (MM)");
  if (cancelled) return;

  waitForPhone();
  lockBox();
  lockEndTime = now() + minutes * 60;
  lockSet = true;
}

// === Display ===
void displayStatus() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  print2Digits(hour());
  lcd.print(":");
  print2Digits(minute());
  lcd.print("      ");

  lcd.setCursor(0, 1);

  if (alarmSet && lockSet) {
    if (millis() - lastToggleTime > 5000) {
      showAlarmLine = !showAlarmLine;
      lastToggleTime = millis();
    }

    if (showAlarmLine) {
      lcd.print("Alarm at: ");
      print2Digits(hour(alarmTime));
      lcd.print(":");
      print2Digits(minute(alarmTime));
      lcd.print(" ");
    } else {
      long secondsLeft = lockEndTime - now();
      if (secondsLeft < 0) secondsLeft = 0;
      int mins = secondsLeft / 60;
      int secs = secondsLeft % 60;
      lcd.print("Unlock in: ");
      print2Digits(mins);
      lcd.print(":");
      print2Digits(secs);
      lcd.print(" ");
    }
  } else if (lockSet) {
    long secondsLeft = lockEndTime - now();
    if (secondsLeft < 0) secondsLeft = 0;
    int mins = secondsLeft / 60;
    int secs = secondsLeft % 60;
    lcd.print("Unlock in: ");
    print2Digits(mins);
    lcd.print(":");
    print2Digits(secs);
    lcd.print(" ");
  } else if (alarmSet) {
    lcd.print("Alarm at: ");
    print2Digits(hour(alarmTime));
    lcd.print(":");
    print2Digits(minute(alarmTime));
    lcd.print(" ");
  } else {
    lcd.print("Mode: Clock     ");
  }
}

// === Locking + Alarm Actions ===
void triggerContinuousAlarm() {
  lcd.clear();
  lcd.print("ALARM ACTIVE!!!");
}

void lockBox() {
  lockServo.write(90);
  isLocked = true;
  lcd.clear();
  lcd.print("Box Locked");
  delay(1000);
}

void unlockBox() {
  lockServo.write(0);
  isLocked = false;
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000);
    delay(150);
    noTone(BUZZER_PIN);
    delay(150);
  }
  lcd.clear();
  lcd.print("Unlocked");
  delay(2000);
}

void waitForPhone() {
  lcd.clear();
  lcd.print("Place phone...");
  while (true) {
    long d = getDistanceCM();
    if (d > 0 && d <= 6) {
      delay(1000);
      if (getDistanceCM() <= 6) break;
    }
  }
}

// === Inputs ===
time_t getTimeFromUser(const char* label) {
  lcd.clear();
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print("HHMM: ");
  String input = "";
  cancelled = false;
  while (input.length() < 4) {
    char k = keypad.getKey();
    if (k == '*') {
      cancelled = true;
      return 0;
    }
    if (k && isDigit(k)) {
      input += k;
      lcd.print(k);
    }
  }
  int h = input.substring(0, 2).toInt();
  int m = input.substring(2, 4).toInt();
  if (h > 23 || m > 59) {
    lcd.clear();
    lcd.print("Invalid time");
    delay(1500);
    return getTimeFromUser(label);
  }
  return previousMidnight(now()) + h * SECS_PER_HOUR + m * SECS_PER_MIN;
}

int getDurationFromUser(const char* label) {
  lcd.clear();
  lcd.print(label);
  lcd.setCursor(0, 1);
  String input = "";
  cancelled = false;
  while (input.length() < 2) {
    char k = keypad.getKey();
    if (k == '*') {
      cancelled = true;
      return 0;
    }
    if (k && isDigit(k)) {
      input += k;
      lcd.print(k);
    }
  }
  return input.toInt();
}

bool confirmAlarmOverride(time_t alarmTime) {
  lcd.clear();
  lcd.print("Override alarm?");
  lcd.setCursor(0, 1);
  lcd.print("Set for ");
  print2Digits(hour(alarmTime));
  lcd.print(":");
  print2Digits(minute(alarmTime));
  delay(2000);

  lcd.clear();
  lcd.print("Press # to OK");
  lcd.setCursor(0, 1);
  lcd.print("* to cancel");
  while (true) {
    char key = keypad.getKey();
    if (key == '#') return true;
    if (key == '*') return false;
  }
}

// === Helpers ===
void print2Digits(int num) {
  if (num < 10) lcd.print("0");
  lcd.print(num);
}

long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void handleAlarmTriggered() {
  // Pulse until phone is detected
  while (true) {
    tone(BUZZER_PIN, 1000);  // on
    delay(200);
    noTone(BUZZER_PIN);  // off
    delay(200);
    long d = getDistanceCM();
    if (d > 0 && d <= 6) {
      noTone(BUZZER_PIN);
      break;
    }
  }

  // Give a moment, then lock
  delay(500);
  lockBox();
  lockEndTime = now() + alarmLockDuration * 60;
  lockSet = true;
  alarmTriggered = false;
  alarmSet = false;
}
