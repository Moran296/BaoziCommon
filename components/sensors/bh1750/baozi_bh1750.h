#ifndef _BOAZI_BH1750_H__
#define _BOAZI_BH1750_H__

#include "bh1750_driver.h"
#include "baozi_sensor.h"
#include "baozi_log.h"
#include "baozi_result.h"
#include "baozi_component.h"

namespace Baozi
{

    class BH1750 : public I_PollingComponent
    {
    public:
        BH1750(HA::Sensor &&sensor);
        void Poll() override;
        eResult Register() override { return m_sensor.Register(); }

    private:
        static constexpr float LUX_TOLERANCE = 10.0;
        static constexpr uint64_t TIME_TOLERANCE = MicroSeconds(Seconds(60)).value();

        BH1750Driver m_driver;
        HA::Sensor m_sensor;
        uint64_t m_lastPublish = 0;
        float m_lastValue = 0;
        bool m_isInitialized = false;

        bool init();
    };

}

#endif