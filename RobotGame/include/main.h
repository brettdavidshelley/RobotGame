#include <Arduino.h>
#include <AlmostRandom.h>

#define RANDOM     AlmostRandom

// TODO: Determine actual BUTTON, TRIG, and ECHO pins.
#define BUTTON_PIN 12
#define TRIG       13
#define ECHO       11

// TODO: Refactor header files maybe
#define TOP        0
#define BOTTOM     1

void               initial_output(void);
void               clear_lcd(void);
char               scan_keypad(void);
void               center_cursor(void);
void               print_centered(String, int)
bool               is_num(char);
void               incorrect(void);
void               correct(void);