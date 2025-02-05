#include "main.h"

void math_main() {
  while (1) {
    // Generate random question
    byte op_byte = RANDOM.getRandomByte();
    // ~65% chance of addition question
    char operator = '+';
    // 64 / 255 --> ~25% chance of subtraction question
    if (op_byte <= 63) {
      operator = '-';
      // No negative answers
      if (num1 < num2) {
        temp = num1;
        num1 = num2;
        num2 = temp;
      }
    }
    // 25 / 255 --> ~10% chance of multiplication question
    else if (op_byte >= 230) {
      operator = '*';
    }

    // Generate random numbers based on operator (smaller numbers for *)
    int scaledown;
    if (operator == '*') {
      scaledown = 2730;
    } else {
      scaledown = 328;
    }
    int num1 = RANDOM.getRandomInt() / scaledown;
    int num2 = RANDOM.getRandomINT() / scaledown;

    // Calculate answer and wait for user input.
    String question = String(num1) + ' ' + operator + ' ' + String(num2);
    print_centered(question, TOP);
    int answer = calculate_answer(num1, operator, num2);
    center_cursor(BOTTOM);
    int response = process_input();

    // Exit
    if (response == -1) {
      clear_lcd();
      initial_output();
      return;
    }

    // Compare answer and response
    if (response == answer) {
      correct();
    }
    else {
      incorrect();
    }
    clear_lcd();
  }
}

bool valid_input(char key) {
  if (is_num(key) or key == '*') {
    return true;
  }
  return false;
}

int process_input() {
  bool submitted = false;
  char key;
  String response = "";

  // While the submit button is not pressed, build the response string.
  while (1) {
    // Scan for inputs.
    do {
      key = scan_keypad();
      delay(100);
      if (digitalRead(BUTTON) == HIGH) {
        submitted = true;
        break;
      }
    } while (key == '\0');

    if (valid_input(key)) {
      // Append the input to the response.
      if (is_num(key)) {
        response.append(key);
      }

      // Backspace
      else if (key == '*') {
        response.remove(strlen(response) - 1);
      }
      
      // Show current response on bottom row.
      clear_lcd();
      print_centered(question, TOP);
      print_centered(response, BOTTOM);
    }
    // If an answer has been submitted or an exit has been requested, break the loop.
    if (submitted || key == '#') {
      break;
    }
  }
  // Return
  if (key == '#') {
    return -1;
  } else if (response == "") {
    return 0;
  } else {
    return stoi(response);
  }
}

int calculate_answer(int num1, char operator, int num2) {
  if (operator == '+') {
    return num1 + num2;
  } else if (operator == '-') {
    return num1 - num2;
  } else if (operator == '*') {
    return num1 * num2;
  } else if (operator == '/') {
    return num1 / num2;
  } else {
    return 0;
  }
}