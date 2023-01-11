#ifndef BAOZI_PRESENCE_H__
#define BAOZI_PRESENCE_H__

#include "baozi_binary_sensor.h"
#include "baozi_gpio.h"
#include "baozi_component.h"

namespace Baozi
{

    class PresenceSensor : public I_PollingComponent
    {
    public:
        PresenceSensor(int pin, HA::BinarySensor &&sensor);
        eResult Register() override;
        void Poll() override;

    private:
        GPI m_gpi;
        HA::BinarySensor m_sensor;
        bool m_lastState = false;
    };

} // namespace Baozi

#endif
