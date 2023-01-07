#include "baozi_device_manager.h"
#include "baozi_connectivity_manager.h"
#include "baozi_time_units.h"
#include "baozi_mdns.h"
#include "baozi_try.h"
#include "baozi_ha_common.h"

#include "secret.h"

namespace Baozi
{

    ConnectivityManager::ConnectivityManager() {}
    void ConnectivityManager::Init()
    {
        connect_wifi();
        connect_mqtt();
    }

    void ConnectivityManager::connect_wifi()
    {
        m_wifi.Init();
        configASSERT(m_wifi.Connect(Secret::WIFI_NAME, Secret::WIFI_PASSWORD) == true);
        uint8_t retries = MAX_WIFI_RECONNECT_TRIES;
        while (!m_wifi.IsConnected())
        {
            BAO_LOG_INFO("connecting...");
            BaoDelay(DeviceManager::WAIT_FOR_CONNECTION_INTERVAL);
            configASSERT(retries-- > 0);
        }
    }

    std::string ConnectivityManager::get_ha_ip()
    {
        Mdns::Init();
        std::string ip;

        bool found = BaoTry{[&ip]()
                            {
                if (auto res = Mdns::FindHomeAssistant(); res.has_value())
                {
                    ip = res.value();
                    return true;
                }

                BAO_LOG_WARNING("home assistant not found! will try again in 10 seconds");
                return false; }}
                         .Times(15)
                         .Delay(10_sec)
                         .Run();

        configASSERT(found);
        return ip;
    }

    void ConnectivityManager::connect_mqtt()
    {
        std::string ip = get_ha_ip();
        std::string name = HA::AddMacToName("baozi");
        strcpy(DEVICE_NAME, name.c_str());

        auto onConnectCallback = [this]()
        {
            BAO_LOG_INFO("Connected to broker!!");
            configASSERT(m_mqtt.Publish(DEVICE_NAME, "online") == eResult::SUCCESS);
        };

        MqttClient::Config config{
            .broker_ip = ip.c_str(),
            .username = Secret::BROKER_USER_NAME,
            .password = Secret::BROKER_PASSWORD,
            .clientId = nullptr,
            .willTopicAndPayload{DEVICE_NAME, "offline"}, // change to device name
            .onConnectCallback = std::move(onConnectCallback)};

        configASSERT(m_mqtt.TryConnect(config) == eResult::SUCCESS);

        bool connected = BaoTry([this]()
                                {
                if (m_mqtt.IsConnected())
                {
                    return true;
                }

                BAO_LOG_WARNING("mqtt not yet connected! will try again in 5 second");
                return false; })
                             .Times(15)
                             .Delay(5_sec)
                             .Run();

        configASSERT(connected);
    }
}
