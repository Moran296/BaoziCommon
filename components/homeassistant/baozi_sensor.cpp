#include "baozi_sensor.h"
#include "baozi_ha_common.h"
#include "baozi_json.h"

namespace Baozi::HA
{
    Sensor::Sensor(const Sensor::Config &config) : m_name(AddMacToName(config.name)),
                                                   m_config(config)
    {
    }

    eResult Sensor::Register()
    {
        auto &mqtt = DeviceManager::GetInstance().GetMqttClient();
        if (not mqtt.IsConnected())
        {
            return eResult::INVALID_STATE;
        }

        std::string value_template = std::string("{{ value_json.") + m_config.state_name + " | round(2) }}";

        BaoJson json{
            KV{"name", m_name},
            KV{"state_topic", state_topic()},
            KV{"value_template", value_template},
            KV{"unit_of_measurement", m_config.unit_of_measurement}};

        if (m_config.device_class != nullptr)
        {
            json.AddVal("device_class", m_config.device_class);
        }

        return mqtt.Publish(config_topic().c_str(), std::move(json));
    }

    std::string Sensor::state_topic()
    {
        return std::string("homeassistant/") + "sensor/" + m_name + "/state";
    }

    std::string Sensor::config_topic()
    {
        return std::string("homeassistant/") + "sensor/" + m_name + "/config";
    }

} // namespace Baozi::HA