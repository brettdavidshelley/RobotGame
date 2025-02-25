#include "main.h"
#include <Ultrasonic.h>

AlmostRandom MUSIC_RANDOM = AlmostRandom();

// Ultrasonic Sensor
#define TRIG             12
#define ECHO             13
#define MIN_CM           2
#define MAX_CM           30
#define MIN_HZ           40
#define MAX_HZ           5000
Ultrasonic ultrasonic(TRIG, ECHO);
int sensor_distance;

// Frequency -> Note Constants
#define SEMI_PER_OCT     12
#define BASE_FREQ        440
const int NOTES_LEN = 7;
const double SCALEDOWN = 255 / (NOTES_LEN - 1);
const char NOTES[NOTES_LEN] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};

// For statistics tracking.
int NOTE_COUNT[NOTES_LEN] = {0, 0, 0, 0, 0, 0, 0};

void music_main() {
  while (1) {
    clear_lcd();

    // Generate random note.
    byte note_byte = MUSIC_RANDOM.getRandomByte();
    if (note_byte < 0) {
      note_byte *= -1;
    }
    int note_index = (int) (note_byte / SCALEDOWN);
    char note = NOTES[note_index];
    NOTE_COUNT[note_index]++;

    String question_top = "Select note: " + String(note);
    String question_bottom = "with the sensor.";
    print_centered(question_top, TOP);
    print_centered(question_bottom, BOTTOM);
    Serial.print("Note: ");
    Serial.println(note);

    // Debug block
    // int sensor_distance = MUSIC_RANDOM.getRandomInt() / (32768 / (MAX_CM - MIN_CM));
    // if (sensor_distance < 0) {
    //   sensor_distance *= -1;
    // }
    // sensor_distance += MIN_CM;
    // int selected_freq = cm_to_freq(sensor_distance);
    // char selected_note = freq_to_note(selected_freq);
    // Serial.println("Sensor Distance: " + String(sensor_distance) + "cm");
    // Serial.println("Selected Frequency: " + String(selected_freq) + "Hz");
    // Serial.println("Selected Note: " + String(selected_note));
    // Serial.println(note == selected_note ? "Correct!\n" : "Incorrect!\n");
    // play_tone(selected_freq, 2000);

    // int sum = 0;
    // for (int i = 0; i < NOTES_LEN; i++) {
    //   sum += NOTE_COUNT[i];
    // }
    // for (int i = 0; i < NOTES_LEN; i++) {
    //   float percent = NOTE_COUNT[i] * 1.0 / sum;
    //   Serial.println(String(NOTES[i]) + ": " + String(percent) + "%");
    // }
    // continue;

    // Await user response.
    char response = process_input_music();
    Serial.print("User Reponse: ");
    Serial.println(response);

    // Exit
    if (response == '#') {
      clear_lcd();
      print_note_stats();
      return;
    }
    // Compare answer and response.
    if (response == note) {
      correct();
    }
    else {
      incorrect();
    }
  }
}

bool valid_distance(int cm) {
  return (cm >= MIN_CM && cm <= MAX_CM);
}

char process_input_music() {
  int selected_freq;
  char selected_note;
  char key;

  while (1) {
    // Check for exit condition.
    key = scan_keypad();
    if (key == '#') {
      return key;
    }

    // Read sensor distance in cm.
    sensor_distance = ultrasonic.read();
    Serial.println("Sensor Distance: " + String(sensor_distance) + "cm");
    if (valid_distance(sensor_distance)) {
      selected_freq = cm_to_freq(sensor_distance);
      selected_note = freq_to_note(selected_freq);
      Serial.println("Selected Frequency: " + String(selected_freq) + "Hz");
      play_tone(selected_freq, 100);
    } else {
      // Ensure invalid distances are incorrect.
      selected_note = 'Z';
    }
    Serial.println("Selected Note: " + String(selected_note));

    // If an answer has been submitted, turn off the speaker and return.
    if (digitalRead(BUTTON) == HIGH) {
      delay(100);
      i2s_stop(I2S_NUM);
      break;
    }
    delay(200);
  }
  return selected_note;
}

int cm_to_freq(int cm) {
  // Map sensor distance in centimeters to a frequency.
  int multiplier = ceil((MAX_HZ - MIN_HZ) * 1.0 / (MAX_CM - MIN_CM));
  return cm * multiplier + MIN_HZ;
}

char freq_to_note(int freq) {
  // Normalize semitone difference to be within the range of natural notes (A, B, C, D, E, F, G)
  int semitoneDifference = (int) (SEMI_PER_OCT * log2(freq * 1.0 / BASE_FREQ));
  int noteIndex = (semitoneDifference % SEMI_PER_OCT + SEMI_PER_OCT) % SEMI_PER_OCT;
  int naturalNoteIndex = noteIndex % NOTES_LEN;
  return NOTES[naturalNoteIndex];
}

void print_note_stats() {
  // Print what percentage of each note was randomly generated for this session.
  int sum = 0;
  for (int i = 0; i < NOTES_LEN; i++) {
    sum += NOTE_COUNT[i];
  }
  Serial.println("Total Notes: " + String(sum));
  for (int i = 0; i < NOTES_LEN; i++) {
    float percent = NOTE_COUNT[i] * 1.0 / sum;
    Serial.print(String(NOTES[i]) + ": " + String(percent) + "%  ");
    NOTE_COUNT[i] = 0;
  }
}