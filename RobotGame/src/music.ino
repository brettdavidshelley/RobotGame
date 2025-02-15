#include "main.h"
#include <Ultrasonic.h>

AlmostRandom MUSIC_RANDOM = AlmostRandom();

#define TRIG             4
#define ECHO             5
#define MIN_CM           0
#define MAX_CM           76
#define MIN_HZ           31
#define MAX_HZ           65535
Ultrasonic ultrasonic(TRIG, ECHO);
int sensor_distance;

void music_main() {
  while (1) {
    clear_lcd();

    // Generate random numbers
    byte note_byte = MUSIC_RANDOM.getRandomByte();
    if (note_byte < 0) {
      note_byte *= -1;
    }
    char note = 'A';

    // Equal chance of A, B, C, D, E, F, or G
    if (note_byte <= 36) {
      note = 'B';
    } else if (note_byte <= 73) {
      note = 'C';
    } else if (note_byte <= 109) {
      note = 'D';
    } else if (note_byte <= 145) {
      note = 'E';
    } else if (note_byte <= 181) {
      note = 'F';
    } else if (note_byte <= 217) {
      note = 'G';
    }

    // Calculate answer and wait for user input.
    String question_top = "Select note: " + String(note);
    String question_bottom = "with the sensor.";
    print_centered(question_top, TOP);
    print_centered(question_bottom, BOTTOM);
    Serial.print("Note: ");
    Serial.println(note);

    char response = process_input_music();
    Serial.print("User Reponse: ");
    Serial.println(response);

    // Exit
    if (response == '#') {
      initial_output();
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
    if (sensor_distance <= MAX_CM) {
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
  int multiplier = (int) ((MAX_HZ - MIN_HZ) / (MAX_CM - MIN_CM));
  return cm * multiplier + MIN_HZ;
}

char freq_to_note(int freq) {
  // TODO: Implement logic to relate frequency to the note being buzzed by the speaker.
  return 'A';
}