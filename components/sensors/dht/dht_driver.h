#ifndef DHT_DRIVER_H
#define DHT_DRIVER_H

#include "driver/gpio.h"

class DHTDriver
{

public:
    enum eDHTResult
    {
        DHT_OK = 0,
        DHT_CHECKSUM_ERROR = -1,
        DHT_TIMEOUT_ERROR = -2
    };

    DHTDriver();

    void setDHTgpio(gpio_num_t gpio);
    void errorHandler(eDHTResult response);
    eDHTResult readDHT();
    float getHumidity();
    float getTemperature();

private:
    gpio_num_t DHTgpio;
    float humidity = 0.;
    float temperature = 0.;

    int getSignalLevel(int usTimeOut, bool state);
};

#endif