#include <cmath>
#include "driver/i2s.h"

#define TOP              0
#define BOTTOM           1

// Speaker
#define I2S_NUM           I2S_NUM_0  // I2S port number
#define I2S_BCK_PIN       2          // Bit Clock (BCK)
#define I2S_LRCK_PIN      4          // Word Select (LRCK)
#define I2S_DATA_PIN      3          // Data In (DIN)

// Button
#define BUTTON            15