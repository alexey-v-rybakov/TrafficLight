#include <Arduino.h>

// проверка
// ========================
// Пины управления
// ========================
#define RED_RELAY_PIN    11
#define GREEN_RELAY_PIN  10
#define BUTTON_PIN       12

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
  
  MODE_RED = 1,
  MODE_GREEN,
  MODE_AUTO,
  MODE_OFF
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
void SetRedState(bool state)
{
  digitalWrite(RED_RELAY_PIN, (!state)?HIGH:LOW);
}
void SetGreenState(bool state)
{
  digitalWrite(GREEN_RELAY_PIN, (!state)?HIGH:LOW);
}

// ========================
void setup() {
  pinMode(RED_RELAY_PIN, OUTPUT);
  pinMode(GREEN_RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  SetRedState(false);
  SetGreenState(false);

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

bool button_processed = false;

// ========================
void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);


  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    button_processed = false;
  }
  

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) 
  {
  
    if ((reading == LOW) && (button_processed == false))
    {
      currentMode = static_cast<Mode>((currentMode % 4) + 1);
      printMode(currentMode);

      resetSignals();
      autoState = AUTO_RED;
      stateStartTime = millis();
      greenState = false;
      button_processed = true;
    }
  }
  
lastButtonState = reading;
  
}

// ========================
void handleMode() {
  switch (currentMode) {
    case MODE_OFF:
      SetRedState(false);
      SetGreenState(false);
      // Всё выключено
      break;

    case MODE_RED:
      SetRedState(true);
      SetGreenState(false);
      break;

    case MODE_GREEN:
      SetRedState(false);
      SetGreenState(true);
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
        SetRedState(false);
        autoState = AUTO_GREEN_BLINK;
        stateStartTime = currentTime;
        greenState = true;
        SetGreenState(true);
        printAutoState(autoState);
      } else {
        SetRedState(true);
        SetGreenState(false);
      }
      break;

    case AUTO_GREEN_BLINK:
      if (currentTime - stateStartTime >= GREEN_BLINK_DURATION) {
        SetGreenState(false);
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
  SetRedState(false);
  SetGreenState(false);
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
