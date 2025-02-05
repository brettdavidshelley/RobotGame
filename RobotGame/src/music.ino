#include <Ultrasonic.h>
#include "main.h"

Ultrasonic ultrasonic(TRIG, ECHO);
int sensor_distance;

void music_main() {
  while (1) {
    // Generate random numbers
    byte note_byte = RANDOM.getRandomByte();
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
    String question_top = "Select note " + note;
    String question_bottom = "using the sensor.";
    print_centered(question_top, TOP);
    print_centered(question_bottom, BOTTOM);
    char response = process_input();

    // Exit
    if (response == '#') {
      clear_lcd();
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
    clear_lcd();
  }
}

char process_input() {
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
    if (sensor_distance <= 400) {
      selected_note = cm_to_note(sensor_distance);
    }

    // If an answer has been submitted, return.
    if (digitalRead(BUTTON) == HIGH) {
      break;
    }
    delay(200);
  }
  return selected_note;
}

char cm_to_note(int cm) {
  // TODO: Implement logic to relate distance in centimeters to the note being buzzed by the speaker.
  return;
}