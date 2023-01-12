#ifndef _BAOZI_CONNECTIVITY_MANAGER_H__
#define _BAOZI_CONNECTIVITY_MANAGER_H__

#include "baozi_wifi.h"
#include "baozi_mqtt.h"
#include "baozi_result.h"

namespace Baozi
{
    class ConnectivityManager
    {
        static constexpr uint8_t MAX_WIFI_RECONNECT_TRIES = 20;

    public:
        ConnectivityManager();
        void Init();

        bool IsConnected() const { return m_wifi.IsConnected() && m_mqtt.IsConnected(); }
        MqttClient &GetMqttClient() { return m_mqtt; }
        const char *GetDeviceName() const { return DEVICE_NAME; }

    private:
        Wifi m_wifi;
        MqttClient m_mqtt;

        void connect_wifi();
        void mdns_init();
        std::string get_ha_ip();
        void connect_mqtt();
        static inline char DEVICE_NAME[32]{};
    };

};

#endif