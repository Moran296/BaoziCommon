#ifndef _DHT_SENSOR_H__
#define _DHT_SENSOR_H__

#include "baozi_sensor.h"
#include "baozi_log.h"
#include "dht_driver.h"
#include "baozi_result.h"
#include "baozi_component.h"

namespace Baozi
{

    class DHTSensor : public I_PollingComponent
    {
    public:
        DHTSensor(int pin, HA::Sensor &&temperatureSensor, HA::Sensor &&humiditySensor);

        void Poll() override;
        eResult Register() override;

    private:
        DHTDriver m_dht;
        HA::Sensor m_temperatureSensor;
        HA::Sensor m_humiditySensor;

        float m_lastTemp = 0;
        float m_lastHumidity = 0;
        uint64_t m_lastTempTimestamp = 0;
        uint64_t m_lastHumidityTimestamp = 0;

        void updateIfChanged(float tolerance, float value, float &lastValue, uint64_t &lastTimestamp, HA::Sensor &sensor);

        static constexpr float HUMIDITY_TOLERANCE = 10.0;
        static constexpr float TEMPARTURE_TOLERANCE = 2.0;
        static constexpr uint64_t TIME_TOLERANCE = MicroSeconds(Seconds(60)).value();
    };

} // namespace Baozi

#endif