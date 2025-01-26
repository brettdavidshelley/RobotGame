#include "esp_log.h"

extern "C"
{
    void app_main() {
        const char* TAG = "MAIN";
        ESP_LOGD(TAG, "Initializing...");
    }
}