#include <Arduino.h>
#include <AlmostRandom.h>
#include "global.h"

#define BUTTON           12

void initial_output(void);
void clear_lcd(void);
char scan_keypad(void);
void center_cursor(void);
void print_centered(String, int);
bool is_num(char);
void incorrect(void);
void correct(void);