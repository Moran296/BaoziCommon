#include "baozi_presence.h"
#include "baozi_log.h"

namespace Baozi
{
    PresenceSensor::PresenceSensor(int pin, HA::BinarySensor &&sensor) : m_gpi(pin),
                                                                         m_sensor(std::move(sensor)) {}

    eResult PresenceSensor::Register()
    {
        eResult res = m_sensor.Register();
        if (res != eResult::SUCCESS)
            return res;

        m_lastState = m_gpi.IsActive();
        return m_sensor.Publish(m_lastState);
    }

    void PresenceSensor::Poll()
    {
        bool state = m_gpi.IsActive();
        if (state != m_lastState)
        {
            m_lastState = state;
            BAO_LOG_INFO("presence sensor detected: - %s", state ? "occupied" : "clear");
            m_sensor.Publish(state);
        }
    }

} // Baozi