#include <Arduino.h>
#include <Wire.h>
#include <SerLCD.h>
#include "games.h"
#include "global.h"


#define INCORRECT        0
#define CORRECT          1

// Servos & Speaker
#define PWM_PER_DEG      255 / 180
#define NO_SERVO         15
#define YES_SERVO        16
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

void init_output_devices() {
  pinMode(NO_SERVO, OUTPUT);
  pinMode(YES_SERVO, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
}

void lcd_init() {
  Wire.begin(SDA, SCL); // Start I2C with SDA on pin 6 and SCL on pin 7
  lcd.begin(Wire); // Initialize LCD over I2C
  lcd.setBacklight(255, 255, 255); // Set the backlight to white
  lcd.setContrast(5); // Set contrast level (adjust as needed)
  clear_lcd();
  lcd.setCursor(0, 0); // Start at the top left of the LCD
}

void clear_lcd() {
  lcd.clear();
  delay(100);
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

// Function to initialize GPIOs for the keypad
void keypad_init() {
    for (int i = 0; i < KEYPAD_NUM_ROWS; i++) {
        pinMode(pin_rows[i], INPUT_PULLUP);  // Enable internal pull-up resistors
    }
    for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
        pinMode(pin_columns[i], OUTPUT);  // Set columns as output
        digitalWrite(pin_columns[i], HIGH);  // Set columns to high initially
    }
}

// Function to scan the keypad and return the key pressed
char scan_keypad() {
    for (int col = 0; col < KEYPAD_NUM_COLS; col++) {
        // Set all columns to HIGH first
        for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
            digitalWrite(pin_columns[i], HIGH);
        }

        // Set the current column to LOW
        digitalWrite(pin_columns[col], LOW);

        // Check for keypress on each row
        for (int row = 0; row < KEYPAD_NUM_ROWS; row++) {
            if (digitalRead(pin_rows[row]) == LOW) { // If row is LOW, key is pressed
                // Wait until the key is released
                while (digitalRead(pin_rows[row]) == LOW) {
                    delay(10);
                }
                return keys[row][col]; // Return the key pressed
            }
        }
    }
    return '\0'; // No key pressed
}

bool is_num(char key) {
  if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4') {
    return true;
  }
  if (key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
    return true;
  }
  return false;
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

void correct() {
  int correct_freq = 1000;
  tone(SPEAKER, correct_freq, TONE_DURATION);
  nod(CORRECT);
  delay(200);
}

void incorrect() {
  int incorrect_freq = 500;
  tone(SPEAKER, incorrect_freq, TONE_DURATION);
  nod(INCORRECT);
  delay(200);
}

void prepare_game() {
  clear_lcd();
  print_centered("*: del, #: esc", TOP);
  print_centered("Button submits.", BOTTOM);
  delay(5000);
  clear_lcd();
}

void initial_output() {
  String opening_str_top = "Math (A) or";
  String opening_str_bottom = "Music (B)? ";
  print_centered(opening_str_top, TOP);
  lcd.setCursor(0, 1);
  lcd.print(opening_str_bottom);
  lcd.cursor();
}

void setup() {
    // Initialize devices
    lcd_init();
    keypad_init();
    init_output_devices();
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
      prepare_game();

      // Math Game
      if (key == 'A') {
        math_main();
        Serial.println("Math game loading...");
      }
      // Music Game
      else if (key == 'B') {
        music_main();
        Serial.println("Music game loading...");
      }
    }
    delay(100);
}
