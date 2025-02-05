#include <Arduino.h>
#include <AlmostRandom.h>
#include "global.h"

// TODO: Determine actual BUTTON, TRIG, and ECHO pins.
#define BUTTON 12
#define TRIG   13
#define ECHO   14

void initial_output(void);
void clear_lcd(void);
char scan_keypad(void);
void center_cursor(void);
void print_centered(String, int);
bool is_num(char);
void incorrect(void);
void correct(void);