#include <Arduino.h>
#include <Wire.h>
#include <SerLCD.h>
#include <Adafruit_SoftServo.h>
#include "games.h"
#include "global.h"

// Results tracking
#define INCORRECT        0
#define CORRECT          1
int questions_answered;
int score;

// Servos & Speaker
#define NO_SERVO         22
#define YES_SERVO        23
#define TONE_DURATION    1000
Adafruit_SoftServo yes_servo;
Adafruit_SoftServo no_servo;

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
  servos_init();
  i2s_speaker_init();
  pinMode(BUTTON, INPUT);
}

void servos_init() {
  pinMode(NO_SERVO, OUTPUT);
  pinMode(YES_SERVO, OUTPUT);
  yes_servo.attach(YES_SERVO);
  no_servo.attach(NO_SERVO);
  yes_servo.write(0);
  no_servo.write(0); 
  delay(10);
}

void i2s_speaker_init() {
  // Configure I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };
  
  // Install and start I2S driver
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

  // Configure I2S pins
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK_PIN,
    .ws_io_num = I2S_LRCK_PIN,
    .data_out_num = I2S_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_set_pin(I2S_NUM, &pin_config);
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

void play_tone(int frequency, int duration) {
  size_t bytes_written;
  const int sample_rate = 44100;
  const int samples_per_cycle = sample_rate / frequency;
  const int amplitude = 8000;

  int16_t samples[samples_per_cycle];
  for (int i = 0; i < samples_per_cycle; i++) {
    samples[i] = amplitude * sin(2 * PI * i / samples_per_cycle);
  }

  // Calculate how many cycles to play for the given duration
  int cycles = (duration / 1000.0) * frequency;
  i2s_start(I2S_NUM);
  for (int c = 0; c < cycles; c++) {
    i2s_write(I2S_NUM, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
  }
  i2s_stop(I2S_NUM);
}

void nod(int correctness) {
  int pos;
  int servo_delay = 5;
  Adafruit_SoftServo servo = (correctness == CORRECT) ? yes_servo : no_servo;

  // Nod up-and-down or side-to-side twice
  for (int i = 0; i < 2; i++) {
    for (pos = 0; pos < 180; pos += 2) {
      servo.write(pos);
      delay(servo_delay);
      if (pos % 5 == 0) {
        servo.refresh();
      }
    }
    for (pos = 179; pos >= 0; pos -= 2) {
      servo.write(pos);
      delay(servo_delay);
      if (pos % 5 == 0) {
        servo.refresh();
      }
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
  play_tone(1800, TONE_DURATION);
  nod(CORRECT);
}

void incorrect() {
  print_correctness("Incorrect!");
  play_tone(500, TONE_DURATION);
  nod(INCORRECT);
}

void loading_screen() {
  clear_lcd();
  print_centered("*: del, #: esc", TOP);
  print_centered(" Button submits.", BOTTOM);
  lcd.noCursor();
  delay(5000);
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
  lcd.noCursor();

  // Reset counters.
  score = 0;
  questions_answered = 0;

  // Enter the main loop again.
  delay(5000);
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
    lcd.noCursor();
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
