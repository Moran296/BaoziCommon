#ifndef _BAOZI_DHT_DRIVER_H__
#define _BAOZI_DHT_DRIVER_H__

#include "baozi_gpio.h"

namespace Baozi::DHT
{
    enum eDHTResult
    {
        DHT_OK = 0,
        DHT_TIMEOUT,
        DHT_CHECKSUM_ERROR,
        DHT_INVALID_ARGUMENT,
        DHT_ERROR
    };

    class Driver
    {
    public:
        Driver(int pin, uint32_t timeout = 1000);
        eDHTResult Read(float &temperature, float &humidity);

    private:
        GPIO m_pin;
        uint32_t m_timeout;
    };
} // namespace Baozi::DHT

#endif