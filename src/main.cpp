#include <Arduino.h>
// проверка
// ========================
// Пины управления
// ========================
#define RED_RELAY_PIN    4
#define GREEN_RELAY_PIN  5
#define BUTTON_PIN       6

// ========================
// Константы
// ========================
#define DEBOUNCE_DELAY       50      // мс
#define BLINK_INTERVAL       1000    // мс
#define RED_DURATION         3000    // мс
#define GREEN_BLINK_DURATION 6000    // мс

// ========================
// Перечисления режимов
// ========================
enum Mode {
  MODE_OFF = 1,
  MODE_RED,
  MODE_GREEN,
  MODE_AUTO
};

enum AutoState {
  AUTO_RED,
  AUTO_GREEN_BLINK
};

// ========================
// Глобальные переменные
// ========================
Mode currentMode = MODE_OFF;
AutoState autoState = AUTO_RED;

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long stateStartTime = 0;
unsigned long lastBlinkTime = 0;
bool greenState = false;

// ========================
// Прототипы функций
// ========================
void handleButton();
void handleMode();
void handleAutoMode();
void resetSignals();
void printMode(Mode mode);
void printAutoState(AutoState state);

// ========================
void setup() {
  pinMode(RED_RELAY_PIN, OUTPUT);
  pinMode(GREEN_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RED_RELAY_PIN, LOW);
  digitalWrite(GREEN_RELAY_PIN, LOW);

  Serial.begin(9600);
  delay(200); // немного подождём запуска монитора порта

  Serial.println("=== Программа управления светофором ===");
  printMode(currentMode);
}

// ========================
void loop() {
  handleButton();
  handleMode();
}

// ========================
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (lastButtonState == HIGH && reading == LOW) {
      currentMode = static_cast<Mode>((currentMode % 4) + 1);
      printMode(currentMode);

      resetSignals();
      autoState = AUTO_RED;
      stateStartTime = millis();
      greenState = false;
    }
  }

  lastButtonState = reading;
}

// ========================
void handleMode() {
  switch (currentMode) {
    case MODE_OFF:
      // Всё выключено
      break;

    case MODE_RED:
      digitalWrite(RED_RELAY_PIN, HIGH);
      digitalWrite(GREEN_RELAY_PIN, LOW);
      break;

    case MODE_GREEN:
      digitalWrite(RED_RELAY_PIN, LOW);
      digitalWrite(GREEN_RELAY_PIN, HIGH);
      break;

    case MODE_AUTO:
      handleAutoMode();
      break;
  }
}

// ========================
void handleAutoMode() {
  unsigned long currentTime = millis();

  switch (autoState) {
    case AUTO_RED:
      if (currentTime - stateStartTime >= RED_DURATION) {
        digitalWrite(RED_RELAY_PIN, LOW);
        autoState = AUTO_GREEN_BLINK;
        stateStartTime = currentTime;
        greenState = false;
        digitalWrite(GREEN_RELAY_PIN, LOW);
        printAutoState(autoState);
      } else {
        digitalWrite(RED_RELAY_PIN, HIGH);
        digitalWrite(GREEN_RELAY_PIN, LOW);
      }
      break;

    case AUTO_GREEN_BLINK:
      if (currentTime - stateStartTime >= GREEN_BLINK_DURATION) {
        digitalWrite(GREEN_RELAY_PIN, LOW);
        autoState = AUTO_RED;
        stateStartTime = currentTime;
        printAutoState(autoState);
      } else {
        if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
          greenState = !greenState;
          digitalWrite(GREEN_RELAY_PIN, greenState ? HIGH : LOW);

          Serial.print(" [AUTO] Зеленый мигает: ");
          Serial.println(greenState ? "ВКЛ" : "ВЫКЛ");

          lastBlinkTime = currentTime;
        }
      }
      break;
  }
}

// ========================
void resetSignals() {
  digitalWrite(RED_RELAY_PIN, LOW);
  digitalWrite(GREEN_RELAY_PIN, LOW);
}

// ========================
void printMode(Mode mode) {
  Serial.print(">> Режим: ");
  switch (mode) {
    case MODE_OFF:   Serial.println("1. Всё выключено"); break;
    case MODE_RED:   Serial.println("2. Горит КРАСНЫЙ"); break;
    case MODE_GREEN: Serial.println("3. Горит ЗЕЛЕНЫЙ"); break;
    case MODE_AUTO:  Serial.println("4. Автоматический цикл"); break;
  }
}

// ========================
void printAutoState(AutoState state) {
  Serial.print("   >> Авто-состояние: ");
  switch (state) {
    case AUTO_RED:         Serial.println("КРАСНЫЙ 3 сек"); break;
    case AUTO_GREEN_BLINK: Serial.println("ЗЕЛЕНЫЙ мигает 6 сек"); break;
  }
}
