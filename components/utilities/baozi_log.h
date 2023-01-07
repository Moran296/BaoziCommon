#ifndef BAOZI_LOG_H__
#define BAOZI_LOG_H__

#include "esp_log.h"

#define LOG_TO_MEMORY(LEVEL, ...) //TODO - implement log to memory flash write

#define BAO_LOG_ERROR(...) \
    ESP_LOGE(__FILE__, ##__VA_ARGS__)

#define BAO_LOG_WARNING(...) \
    ESP_LOGW(__FILE__, ##__VA_ARGS__)

#define BAO_LOG_INFO(...) \
    ESP_LOGI(__FILE__, ##__VA_ARGS__)

#define BAO_LOG_DEBUG(...) ESP_LOGD(__FILE__, ##__VA_ARGS__)

#endif