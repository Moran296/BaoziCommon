#ifndef BAO_HA_SENSOR_H__
#define BAO_HA_SENSOR_H__

// For possible device classes and their presentation: https://www.home-assistant.io/integrations/sensor/#device-class

#include <string>
#include "baozi_json_traits.h"
#include "baozi_mqtt.h"
#include "baozi_result.h"
#include "baozi_device_manager.h"
#include <math.h>

namespace Baozi::HA
{
    class Sensor
    {
    public:
        // see examples at the bottom of this file
        struct Config
        {
            const char *name;
            const char *unit_of_measurement;
            const char *device_class;
            const char *state_name;
        };

        Sensor(const Config &config);

        eResult Register();

        template <typename T>
            requires is_json_serializable_v<T>
        eResult Publish(T value)
        {
            auto &mqtt = DeviceManager::GetInstance().GetMqttClient();
            if (not mqtt.IsConnected())
            {
                return eResult::INVALID_STATE;
            }

            BaoJson json = {
                KV{m_config.state_name, value}};

            return mqtt.Publish(state_topic().c_str(), std::move(json));
        }

    private:
        std::string m_name;
        const Config &m_config;

        std::string state_topic();
        std::string config_topic();
    };

    static inline constexpr const char *_temperature_name = "temperature";
    static inline constexpr Sensor::Config TEMPERATURE_SENSOR_CONFIG = {
        .name = _temperature_name,
        .unit_of_measurement = "°C",
        .device_class = _temperature_name,
        .state_name = _temperature_name};

    static inline constexpr const char *_humidity_name = "humidity";
    static inline constexpr Sensor::Config HUMIDITY_SENSOR_CONFIG = {
        .name = _humidity_name,
        .unit_of_measurement = "%",
        .device_class = _humidity_name,
        .state_name = _humidity_name};

    static inline constexpr Sensor::Config LIGHT_SENSOR_CONFIG = {
        .name = "light",
        .unit_of_measurement = "lx",
        .device_class = "illuminance",
        .state_name = "light"};

    static inline constexpr const char *_battery_name = "battery";
    static inline constexpr Sensor::Config BATTERY_SENSOR_CONFIG = {
        .name = _battery_name,
        .unit_of_measurement = "%",
        .device_class = _battery_name,
        .state_name = _battery_name};

    static inline constexpr const char *_sound_name = "sound";
    static inline constexpr Sensor::Config SOUND_SENSOR_CONFIG = {
        .name = _sound_name,
        .unit_of_measurement = "dB",
        .device_class = "sound_pressure",
        .state_name = _sound_name};

} // namespace Baozi::HA

#endif