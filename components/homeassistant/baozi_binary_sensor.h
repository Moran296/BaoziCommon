#ifndef BAO_HA_BINARY_SENSOR_H__
#define BAO_HA_BINARY_SENSOR_H__

#include "baozi_mqtt.h"
#include "baozi_ha_common.h"

// For possible device classes and their presentation: https://www.home-assistant.io/integrations/binary_sensor/#device-class

namespace Baozi::HA
{

    class BinarySensor
    {

    public:
        BinarySensor(const char *name, const char *device_class = nullptr);

        eResult Register();
        eResult Publish(bool state);

        void SetIcon(const char *icon);

    private:
        std::string m_name;
        const char *m_device_class;
        const char *m_icon = SWITCH_ICON;

        static inline constexpr const char *s_state_name = "state";
        static inline constexpr const char *s_unit_of_measurement = "";

        std::string state_topic();
        std::string config_topic();
    };

} // namespace Baozi

#endif