#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Servo pillServo;

// Pins
const int buzzerPin = 7;
const int irSensorPin = 8;
const int servoPin = 6;

// Alarm Times for Pills
const int pill1Hour = 22;
const int pill1Minute = 40;
const int pill1Second = 30;

const int pill2Hour = 22;
const int pill2Minute = 41;
const int pill2Second = 30;

const int pill3Hour = 22;
const int pill3Minute = 42;
const int pill3Second = 30;

// Flags
bool pill1Given = false, pill2Given = false, pill3Given = false;
bool pill1Reminder = false, pill2Reminder = false, pill3Reminder = false;
unsigned long pill1ReminderStart = 0, pill2ReminderStart = 0, pill3ReminderStart = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(buzzerPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  pillServo.attach(servoPin);
  pillServo.write(0);

  if (!rtc.begin()) {
    lcd.print("RTC not found!");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Pill Box");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();
  int h = now.hour();
  int m = now.minute();
  int s = now.second();

  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  printTime(h, m, s);

  // Pill 1
  if (h == pill1Hour && m == pill1Minute && s == pill1Second && !pill1Given) {
    runBuzzerAndWait(1, 55);
  }

  if (pill1Reminder && millis() - pill1ReminderStart >= 300000 && !pill1Given) {
    lcd.setCursor(0, 1);
    lcd.print("Reminder! Pill 1");
    runBuzzerAndWait(1, 55);
    pill1Reminder = false;
  }

  // Pill 2
  if (h == pill2Hour && m == pill2Minute && s == pill2Second && !pill2Given) {
    runBuzzerAndWait(2, 110);
  }

  if (pill2Reminder && millis() - pill2ReminderStart >= 300000 && !pill2Given) {
    lcd.setCursor(0, 1);
    lcd.print("Reminder! Pill 2");
    runBuzzerAndWait(2, 110);
    pill2Reminder = false;
  }

  // Pill 3
  if (h == pill3Hour && m == pill3Minute && s == pill3Second && !pill3Given) {
    runBuzzerAndWait(3, 180);
  }

  if (pill3Reminder && millis() - pill3ReminderStart >= 300000 && !pill3Given) {
    lcd.setCursor(0, 1);
    lcd.print("Reminder! Pill 3");
    runBuzzerAndWait(3, 180);
    pill3Reminder = false;
  }

  delay(1000);
}

// Buzzer & Pill Dispenser Logic
void runBuzzerAndWait(int pillNumber, int angle) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time for Pill ");
  lcd.print(pillNumber);
  lcd.setCursor(0, 1);
  lcd.print("Buzzer Ringing!");

  Serial.print("🔔 Pill ");
  Serial.print(pillNumber);
  Serial.println(" Alarm!");

  unsigned long buzzerStart = millis();

  while (millis() - buzzerStart <= 30000) {
    digitalWrite(buzzerPin, HIGH);
    delay(300);
    digitalWrite(buzzerPin, LOW);
    delay(300);

    if (stablePersonDetected()) {
      Serial.println("✅ Person Detected!");
      Serial.println("⚙️ Servo Rotated!");
      Serial.println("👋 Take Pill!");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Person Here");
      lcd.setCursor(0, 1);
      lcd.print("Pill Dispensed");

      pillServo.write(angle);
      delay(6000);
      pillServo.write(0);
      delay(500);

      markPillGiven(pillNumber);
      lcd.clear();
      return;
    }
  }

  digitalWrite(buzzerPin, LOW);
  lcd.clear();
  if (!isPillGiven(pillNumber)) {
    setReminder(pillNumber);
    Serial.println("⏳ Reminder set for 5 min");
  }
}

// Helper Functions
void markPillGiven(int pillNumber) {
  if (pillNumber == 1) pill1Given = true;
  if (pillNumber == 2) pill2Given = true;
  if (pillNumber == 3) pill3Given = true;
}

bool isPillGiven(int pillNumber) {
  if (pillNumber == 1) return pill1Given;
  if (pillNumber == 2) return pill2Given;
  if (pillNumber == 3) return pill3Given;
  return false;
}

void setReminder(int pillNumber) {
  if (pillNumber == 1) {
    pill1Reminder = true;
    pill1ReminderStart = millis();
  }
  if (pillNumber == 2) {
    pill2Reminder = true;
    pill2ReminderStart = millis();
  }
  if (pillNumber == 3) {
    pill3Reminder = true;
    pill3ReminderStart = millis();
  }
}

// Person Detection
bool stablePersonDetected() {
  int detectCount = 0;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(irSensorPin) == LOW) {
      detectCount++;
    }
    delay(20);
  }
  return detectCount >= 8;
}

// Display Time
void printTime(int h, int m, int s) {
  if (h < 10) lcd.print("0");
  lcd.print(h); lcd.print(":");
  if (m < 10) lcd.print("0");
  lcd.print(m); lcd.print(":");
  if (s < 10) lcd.print("0");
  lcd.print(s);
}
