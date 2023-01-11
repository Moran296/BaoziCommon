#include "baozi_dht22.h"
#include <esp_timer.h>

namespace Baozi
{
    DHTSensor::DHTSensor(int pin, HA::Sensor &&temperatureSensor, HA::Sensor &&humiditySensor)
        : m_temperatureSensor(std::move(temperatureSensor)),
          m_humiditySensor(std::move(humiditySensor))
    {
        m_dht.setDHTgpio((gpio_num_t)pin);
    }

    void DHTSensor::updateIfChanged(float tolerance, float value, float &lastValue, uint64_t &lastTimestamp, HA::Sensor &sensor)
    {
        uint64_t now = esp_timer_get_time();
        uint64_t timePassed = now - lastTimestamp;

        bool toleranceMet = abs(value - lastValue) > tolerance;
        bool timeToleranceMet = timePassed > TIME_TOLERANCE;

        if (toleranceMet || timeToleranceMet)
        {
            lastValue = value;
            lastTimestamp = esp_timer_get_time();
            sensor.Publish(value);
        }
    }

    void DHTSensor::Poll()
    {
        DHTDriver::eDHTResult result = m_dht.readDHT();
        if (result != DHTDriver::DHT_OK)
        {
            BAO_LOG_ERROR("DHT read failed: %d", (int)result);
            return;
        }

        updateIfChanged(HUMIDITY_TOLERANCE,
                        m_dht.getHumidity(),
                        m_lastHumidity,
                        m_lastHumidityTimestamp,
                        m_humiditySensor);

        updateIfChanged(TEMPARTURE_TOLERANCE,
                        m_dht.getTemperature(),
                        m_lastTemp,
                        m_lastTempTimestamp,
                        m_temperatureSensor);
    }

    eResult DHTSensor::Register()
    {
        if (auto res = m_temperatureSensor.Register(); res != eResult::SUCCESS)
        {
            return res;
        }

        if (auto res = m_humiditySensor.Register(); res != eResult::SUCCESS)
        {
            return res;
        }

        return eResult::SUCCESS;
    }

}