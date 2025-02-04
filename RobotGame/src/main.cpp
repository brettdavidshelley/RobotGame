#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// UART settings for 4x20 LCD
#define UART_NUM           UART_NUM_1
#define UART_BAUD_RATE     9600
#define UART_TX_PIN        17  // ESP32 TX pin connected to LCD RX
#define UART_RX_PIN        16  // ESP32 RX pin connected to LCD TX
#define UART_BUF_SIZE      1024
#define LCD_NUM_ROWS       2
#define LCD_NUM_COLS       16

// 4x4 Keypad
#define KEYPAD_NUM_ROWS  4
#define KEYPAD_NUM_COLS  4
char keys[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
gpio_num_t pin_rows[KEYPAD_NUM_ROWS] = {GPIO_NUM_21, GPIO_NUM_20, GPIO_NUM_19, GPIO_NUM_18};
gpio_num_t pin_columns[KEYPAD_NUM_COLS] = {GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9};

// UART driver initialization
void uart_init() {
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, UART_BUF_SIZE, 0, 0, NULL, 0));
}

// Send data to the LCD over UART
void lcd_write_byte(uint8_t data) {
    uart_write_bytes(UART_NUM, (const char*)&data, 1);
}

// Clear the LCD screen
void lcd_clear() {
    lcd_write_byte(0x01);  // Clear display
    vTaskDelay(pdMS_TO_TICKS(2)); // Wait for the command to be processed
}

// Initialize the LCD (sending basic commands over UART)
void lcd_init() {
    lcd_write_byte(0x33);  // Initialize
    lcd_write_byte(0x32);  // Initialize
    lcd_write_byte(0x06);  // Entry mode
    lcd_write_byte(0x0C);  // Display on, cursor off
    lcd_write_byte(0x28);  // 4-bit mode, 2-line display
    lcd_clear();
}

// Display a string on the LCD over UART
void lcd_display_string(const char *str) {
    while (*str) {
        lcd_write_byte(*str++);
    }
}

// Function to initialize GPIOs for the keypad
void keypad_init() {
    for (int i = 0; i < KEYPAD_NUM_ROWS; i++) {
        gpio_set_direction(pin_rows[i], GPIO_MODE_INPUT);
        gpio_pullup_en(pin_rows[i]); // Enable pull-up resistors
    }
    for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
        gpio_set_direction(pin_columns[i], GPIO_MODE_OUTPUT);
        gpio_set_level(pin_columns[i], 1); // Set columns to high initially
    }
}

// Function to scan the keypad and return the key pressed
char scan_keypad() {
    for (int col = 0; col < KEYPAD_NUM_COLS; col++) {
        // Set all columns to HIGH first
        for (int i = 0; i < KEYPAD_NUM_COLS; i++) {
            gpio_set_level(pin_columns[i], 1);
        }

        // Set the current column to LOW
        gpio_set_level(pin_columns[col], 0);

        // Check for keypress on each row
        for (int row = 0; row < KEYPAD_NUM_ROWS; row++) {
            if (gpio_get_level(pin_rows[row]) == 0) { // If row is LOW, key is pressed
                // Wait until the key is released
                while (gpio_get_level(pin_rows[row]) == 0) {
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
                return keys[row][col]; // Return the key pressed
            }
        }
    }
    return '\0'; // No key pressed
}

extern "C" void app_main() {
    // Initialize devices
    keypad_init();
    uart_init();
    lcd_init();
    printf("All devices initialized!\n");

    int key_buf_len = 16;
    char key_buf[key_buf_len];
    int key_idx = 0;
    int row_idx = 0;
    int col_idx = 0;
    while (1) {
        char key = scan_keypad();
        if (key != '\0') {
            printf("Key pressed: %c\n", key);

            // If the LCD is full, clear and restart.
            if (row_idx == LCD_NUM_ROWS - 1) {
                lcd_clear();
                col_idx = 0;
                row_idx = 0;
            }
            // If the row is full, increment to the next row.
            else if (col_idx == LCD_NUM_COLS - 1) {
                col_idx = 0;
                row_idx++;
            }
            // Add the key to the buffer
            if (key_idx == key_buf_len - 1) {
                key_idx = 0;
            }
            key_buf[key_idx++] = key;

            // Display on the LCD
            key_buf[key_idx] = '\0'; // Null-terminate the string
            lcd_display_string(key_buf);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for a while before scanning again
    }
}