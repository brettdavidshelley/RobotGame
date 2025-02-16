#include "main.h"
#include <cmath>
#include <Ultrasonic.h>

AlmostRandom MUSIC_RANDOM = AlmostRandom();

// Ultrasonic Sensor
#define TRIG             4
#define ECHO             5
#define MIN_CM           0
#define MAX_CM           76
#define MIN_HZ           31
#define MAX_HZ           32767
Ultrasonic ultrasonic(TRIG, ECHO);
int sensor_distance;

// Frequency -> Note Constants
#define SEMI_PER_OCT     12
#define BASE_FREQ        440
const int NOTES_LEN = 7;
const double SCALEDOWN = 255 / (NOTES_LEN - 1);
const char NOTES[NOTES_LEN] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};

void music_main() {
  while (1) {
    clear_lcd();

    // Generate random note.
    byte note_byte = MUSIC_RANDOM.getRandomByte();
    if (note_byte < 0) {
      note_byte *= -1;
    }

    // char note = 'A';
    // // Equal chance of A, B, C, D, E, F, or G
    // if (note_byte <= 36) {
    //   note = 'B';
    // } else if (note_byte <= 73) {
    //   note = 'C';
    // } else if (note_byte <= 109) {
    //   note = 'D';
    // } else if (note_byte <= 145) {
    //   note = 'E';
    // } else if (note_byte <= 181) {
    //   note = 'F';
    // } else if (note_byte <= 217) {
    //   note = 'G';
    // }

    int note_index = (int) (note_byte / SCALEDOWN);
    char note = NOTES[note_index];

    // Calculate answer and wait for user input.
    String question_top = "Select note: " + String(note);
    String question_bottom = "with the sensor.";
    print_centered(question_top, TOP);
    print_centered(question_bottom, BOTTOM);
    Serial.print("Note: ");
    Serial.println(note);

    // Debug block
    // int sensor_distance = MUSIC_RANDOM.getRandomInt() / 430;
    // if (sensor_distance < 0) {
    //   sensor_distance *= -1;
    // }
    // int selected_freq = cm_to_freq(sensor_distance);
    // char selected_note = freq_to_note(selected_freq);
    // Serial.println("Sensor Distance: " + String(sensor_distance) + "cm");
    // Serial.println("Selected Frequency: " + String(selected_freq) + "Hz");
    // Serial.println("Selected Note: " + String(selected_note));
    // Serial.println(note == selected_note ? "Correct!\n" : "Incorrect!\n");
    // delay(250);
    // continue;

    char response = process_input_music();
    Serial.print("User Reponse: ");
    Serial.println(response);

    // Exit
    if (response == '#') {
      clear_lcd();
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
    if (valid_distance(sensor_distance)) {
      selected_freq = cm_to_freq(sensor_distance);
      selected_note = freq_to_note(selected_freq);
      tone(SPEAKER, selected_freq);
    }

    // If an answer has been submitted, return.
    if (digitalRead(BUTTON) == HIGH) {
      delay(100);
      noTone(SPEAKER);
      break;
    }
    delay(100);
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