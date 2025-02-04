#include <Arduino.h>
#include <Wire.h>
#include <SerLCD.h>

// 16x2 LCD
#define LCD_NUM_ROWS     2
#define LCD_NUM_COLS     16
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

void lcd_init() {
  Wire.begin();
  lcd.begin(Wire);
  lcd.setBacklight(255, 255, 255);
  lcd.setContrast(5);
  lcd.setCursor(0, 0);
}

void clear_lcd() {
  lcd.clear();
  delay(200);
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

void setup() {
    // Initialize devices
    lcd_init();
    keypad_init();
    Serial.begin(9600);
}

void loop() {
    int row_idx = 0;
    int col_idx = 0;
    
    char key = scan_keypad();
    if (key != '\0') {
        // If the LCD is full, clear and restart.
        if (row_idx == LCD_NUM_ROWS - 1 && col_idx == LCD_NUM_COLS - 1) {
            clear_lcd();
            col_idx = 0;
            row_idx = 0;
        }
        // If the row is full, increment to the next row.
        else if (col_idx == 15) {
            col_idx = 0;
            row_idx++;
        }
        lcd.setCursor(col_idx++, row_idx);
        Serial.print(key);
    }
    delay(100); // Delay for a while before scanning again
}