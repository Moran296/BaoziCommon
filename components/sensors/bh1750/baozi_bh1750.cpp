#include "baozi_bh1750.h"
#include <esp_timer.h>

namespace Baozi
{

    BH1750::BH1750(HA::Sensor &&sensor) : m_sensor(std::move(sensor))
    {
        eResult res = m_driver.PowerOn();
        if (res != eResult::SUCCESS)
            BAO_LOG_ERROR("BH1750 failed powering on: %d", static_cast<int>(res));

        res = m_driver.SetMode(BH1750Driver::eMode::BH1750_CONTINUE_1LX_RES);
        if (res != eResult::SUCCESS)
            BAO_LOG_ERROR("BH1750 failed setting mode: %d", static_cast<int>(res));

        BAO_LOG_WARNING("BH1750 initialized");
    }

    void BH1750::Poll()
    {
        auto result = m_driver.GetData();
        if (!result.has_value())
        {
            BAO_LOG_ERROR("BH1750 failed reading: %d", static_cast<int>(result.error()));
            return;
        }

        float value = result.value();
        uint64_t now = esp_timer_get_time();
        uint64_t timePassed = now - m_lastPublish;

        bool toleranceMet = abs(value - m_lastValue) > LUX_TOLERANCE;
        bool timeToleranceMet = timePassed > TIME_TOLERANCE;

        if (toleranceMet || timeToleranceMet)
        {
            m_lastValue = value;
            m_lastPublish = esp_timer_get_time();
            m_sensor.Publish(value);
        }
    }

};
