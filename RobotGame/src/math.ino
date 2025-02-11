#include "main.h"

AlmostRandom MATH_RANDOM = AlmostRandom();
String question;

void math_main() {
  while (1) {
    // Generate random question
    byte op_byte = MATH_RANDOM.getRandomByte();
    // ~65% chance of addition question
    char op = '+';
    // 64 / 255 --> ~25% chance of subtraction question
    if (op_byte <= 63) {
      op = '-';
    }
    // 25 / 255 --> ~10% chance of multiplication question
    else if (op_byte >= 230) {
      op = '*';
    }

    // Generate random numbers based on operator (smaller numbers for *)
    int scaledown;
    if (op == '*') {
      scaledown = 2730;
    } else {
      scaledown = 328;
    }
    int num1 = MATH_RANDOM.getRandomInt() / scaledown;
    int num2 = MATH_RANDOM.getRandomInt() / scaledown;

    if (op == '-' && num1 < num2) {
      int temp = num1;
      num1 = num2;
      num2 = temp;
    }

    // Calculate answer and wait for user input.
    question = String(num1) + ' ' + op + ' ' + String(num2);
    Serial.print("Question: ");
    Serial.println(question);
    print_centered(question, TOP);

    int answer = calculate_answer(num1, op, num2);
    Serial.print("Answer: ");
    Serial.println(answer);

    center_cursor(BOTTOM);
    int response = process_input_math();
    Serial.print("User Response: ");
    Serial.println(response);

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
  if (is_num(key) || key == '*') {
    return true;
  }
  return false;
}

int process_input_math() {
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
        response += String(key);
      }

      // Backspace
      else if (key == '*') {
        response.remove(strlen(response.c_str()) - 1);
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
    return response.toInt();
  }
}

int calculate_answer(int num1, char op, int num2) {
  if (op == '+') {
    return num1 + num2;
  } else if (op == '-') {
    return num1 - num2;
  } else if (op == '*') {
    return num1 * num2;
  } else if (op == '/') {
    return num1 / num2;
  } else {
    return 0;
  }
}