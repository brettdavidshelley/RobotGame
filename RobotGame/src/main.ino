#include <Arduino.h>
#include <Wire.h>
#include <SerLCD.h>
#include "games.h"
#include "global.h"

// Results tracking
#define INCORRECT        0
#define CORRECT          1
int questions_answered;
int score;

// Servos & Speaker
#define PWM_PER_DEG      255 / 180
#define NO_SERVO         22
#define YES_SERVO        23
#define TONE_DURATION    1200

// 16x2 LCD
#define LCD_NUM_ROWS     2
#define LCD_NUM_COLS     16
#define SCL              7
#define SDA              6
SerLCD lcd;

// 4x4 Keypad
#define KEYPAD_NUM_ROWS  4
#define KEYPAD_NUM_COLS  4
char keys[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
int pin_rows[KEYPAD_NUM_ROWS] = {21, 20, 19, 18};
int pin_columns[KEYPAD_NUM_COLS] = {10, 11, 8, 9};

void init_devices() {
  lcd_init();
  keypad_init();
  pinMode(BUTTON, INPUT);
  pinMode(NO_SERVO, OUTPUT);
  pinMode(YES_SERVO, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
}

void lcd_init() {
  // Initialize LCD over I2C.
  Wire.begin(SDA, SCL);
  lcd.begin(Wire);
  lcd.setBacklight(255, 255, 255);
  clear_lcd();
  lcd.setCursor(0, 0);
}

void clear_lcd() {
  lcd.clear();
  delay(50);
}

void print_centered(String message, int row) {
  if (message.length() < LCD_NUM_COLS) {
    lcd.setCursor((LCD_NUM_COLS - message.length()) >> 1, row);
  } else {
    lcd.setCursor(0, row);
  }
  lcd.print(message);
}

void center_cursor(int row) {
  lcd.setCursor((LCD_NUM_COLS - 1) / 2, row);
  lcd.cursor();
}

// Initialize pins for the keypad and the button.
void keypad_init() {
    for (int i = 0; i < KEYPAD_NUM_ROWS; i++) {
        // Enable internal pull-up resistors
        pinMode(pin_rows[i], INPUT_PULLUP);
    }
    for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
        pinMode(pin_columns[i], OUTPUT);
        digitalWrite(pin_columns[i], HIGH);
    }
}

// Function to scan the keypad and return the key pressed
char scan_keypad() {
    for (int col = 0; col < KEYPAD_NUM_COLS; col++) {
        // Set all columns to HIGH first.
        for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
            digitalWrite(pin_columns[i], HIGH);
        }
        // Set the current column to LOW.
        digitalWrite(pin_columns[col], LOW);
        // Check for keypress on each row.
        for (int row = 0; row < KEYPAD_NUM_ROWS; row++) {
            // Key pressed
            if (digitalRead(pin_rows[row]) == LOW) {
                // Wait until the key is released.
                while (digitalRead(pin_rows[row]) == LOW) {
                    delay(10);
                }
                return keys[row][col];
            }
        }
    }
    return '\0';
}

bool is_num(char key) {
  return (key >= 48 && key <= 57);
}

void nod(int correctness) {
  int pos;
  int servo_pin;
  int servo_delay = 20;
  if (correctness == CORRECT) {
    servo_pin = YES_SERVO;
  } else {
    servo_pin = NO_SERVO;
  }
  for (int i = 0; i < 2; i++) {
    for (pos = 0; pos <= PWM_PER_DEG * 180; pos += PWM_PER_DEG) {
      analogWrite(servo_pin, (int) pos);
      delay(servo_delay);
    }
    for (pos = PWM_PER_DEG * 180; pos >= 0; pos -= PWM_PER_DEG) {
      analogWrite(servo_pin, (int) pos);
      delay(servo_delay);
    }
  }
}

void print_correctness(String correctness_str) {
  Serial.println(correctness_str);
  lcd.noCursor();
  clear_lcd();
  print_centered(correctness_str, TOP);
  questions_answered += 1;
  delay(1000);
}

void correct() {
  print_correctness("Correct!");
  score += 1;
  // TODO: REMOVE LATER
  return;

  int correct_freq = 1000;
  tone(SPEAKER, correct_freq, TONE_DURATION);
  nod(CORRECT);
}

void incorrect() {
  print_correctness("Incorrect!");
  // TODO: REMOVE LATER
  return;

  int incorrect_freq = 500;
  tone(SPEAKER, incorrect_freq, TONE_DURATION);
  nod(INCORRECT);
}

void loading_screen() {
  clear_lcd();
  print_centered("*: del, #: esc", TOP);
  print_centered("Button submits.", BOTTOM);
  // TODO: LENGTHEN TO 5000 LATER
  delay(500);
  clear_lcd();
}

void initial_output() {
  // Main loop LCD output
  String opening_str_top = "Math (A) or";
  String opening_str_bottom = "Music (B)? ";
  print_centered(opening_str_top, TOP);
  lcd.setCursor(1, 1);
  lcd.print(opening_str_bottom);
  lcd.cursor();
}

void display_results() {
  // Calculate accuracy.
  int accuracy = 0;
  if (score > 0) {
    accuracy = score * 100 / questions_answered;
  }
  String acc_str = "Accuracy: " + String(accuracy) + "%";

  // Print the results to the console.
  Serial.println("\nResults");
  Serial.println("Questions answered: " + String(questions_answered));
  Serial.println("Questions correct: " + String(score));
  Serial.println(acc_str);

  // Show results of the game.
  String results_str_top = "Score: " + String(score);
  print_centered(results_str_top, TOP);
  print_centered(acc_str, BOTTOM);

  // Reset counters.
  score = 0;
  questions_answered = 0;

  // Enter the main loop again.
  // TODO: LENGTHEN TO 5000 LATER
  delay(2000);
  clear_lcd();
  initial_output();
}

void setup() {
    // Initialize devices.
    init_devices();
    initial_output();
    Serial.begin(9600);
    while (!Serial);
    Serial.println("Initialization complete!");
}

void loop() {
    char key = scan_keypad();
    if (key == 'A' or key == 'B') {
      lcd.print(String(key));
      delay(500);
      loading_screen();

      // Math Game
      if (key == 'A') {
        Serial.println("Math game loading...");
        math_main();
      }
      // Music Game
      else if (key == 'B') {
        Serial.println("Music game loading...");
        music_main();
      }
      display_results();
    }
    delay(100);
}
