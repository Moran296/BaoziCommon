#include "baozi_binary_sensor.h"
#include "baozi_device_manager.h"
#include "baozi_json.h"

namespace Baozi::HA
{

    BinarySensor::BinarySensor(const char *name, const char *device_class) : m_name(AddDeviceNamePrefix(name)),
                                                                             m_device_class(device_class) {}

    eResult BinarySensor::Register()
    {
        auto &mqtt = DeviceManager::GetInstance().GetMqttClient();
        if (not mqtt.IsConnected())
        {
            return eResult::INVALID_STATE;
        }

        std::string value_template = std::string("{{ value_json.") + s_state_name + " }}";
        BaoJson json = {
            KV{"name", m_name},
            KV{"state_topic", state_topic()},
            KV{"value_template", value_template},
            KV{"unit_of_measurement", s_unit_of_measurement},
            KV{"icon", m_icon},
            KV{"on_payload", "ON"},
            KV{"off_payload", "OFF"}};

        if (m_device_class != nullptr)
        {
            json.AddVal("device_class", m_device_class);
        }

        return mqtt.Publish(config_topic().c_str(), json);
    }

    eResult BinarySensor::Publish(bool state)
    {
        auto &mqtt = DeviceManager::GetInstance().GetMqttClient();
        if (not mqtt.IsConnected())
        {
            return eResult::INVALID_STATE;
        }

        BaoJson json{
            KV{s_state_name, state ? "ON" : "OFF"}};

        auto topic = state_topic();
        return mqtt.Publish(topic.c_str(), json);
    }

    void BinarySensor::SetIcon(const char *icon)
    {
        m_icon = icon;
    }

    std::string BinarySensor::state_topic()
    {
        return std::string("homeassistant/") + "binary_sensor/" + m_name + "/state";
    }

    std::string BinarySensor::config_topic()
    {
        return std::string("homeassistant/") + "binary_sensor/" + m_name + "/config";
    }

} // namespace Baozi::HA