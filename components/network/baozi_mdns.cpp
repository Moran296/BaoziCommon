#include "baozi_mdns.h"
#include "mdns.h"
#include "esp_wifi.h"
#include <esp_log.h>

#define MQTT_SRV "_mqtt"
#define MQTT_PROTO "_tcp"
#define DEVICE_NAME "BAOZI" // TODO get from config

namespace Baozi
{

    bool Mdns::Init()
    {
        esp_err_t err = mdns_init();
        if (err != ESP_OK)
        {
            ESP_LOGW("MDNS", "mdns server init failed. err %d", err);
            return false;
        }

        if (mdns_hostname_set(DEVICE_NAME) != ESP_OK)
        {
            ESP_LOGW("MDNS", "mdns hostname set failed");
            return false;
        }

        return true;
    }

    void Mdns::AdvertiseMqtt()
    {

        if (mdns_service_add(MQTT_SRV, MQTT_PROTO, "_tcp", 1883, NULL, 0) != ESP_OK)
        {
            ESP_LOGW("MDNS", "mdns service add failed");
            // return eResult::ERROR_GENERAL;
        }

        // return eResult::SUCCESS;
    }

    void macToString(uint8_t *mac, char *str)
    {
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    void Mdns::AdvertiseESPNOW()
    {

        uint8_t mac[6]{};
        char stringedMac[18]{};
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        macToString(mac, stringedMac);

        mdns_txt_item_t address = {"mac", stringedMac};
        if (mdns_service_add("_espnow", "_espnow", "_tcp", 4080, &address, 1) != ESP_OK)
        {
            ESP_LOGW("MDNS", "mdns service add failed");
            // return eResult::ERROR_GENERAL;
        }

        // return eResult::SUCCESS;
    }

    std::optional<const char *> Mdns::FindBroker()
    {
        mdns_result_t *results = nullptr;
        esp_err_t err = mdns_query_ptr(MQTT_SRV, MQTT_PROTO, 1500, 1, &results);
        if (err != ESP_OK)
        {
            ESP_LOGW("MDNS", "couldn't find broker. err %d", err);
            // return eResult::ERROR_CONNECTION_FAILURE;
            return std::nullopt;
        }
        if (results == nullptr)
        {
            ESP_LOGW("MDNS", "couldn't find broker. no results");
            // return eResult::ERROR_NOT_FOUND;
            return std::nullopt;
        }

        static char address[20]{};
        snprintf(address, 20, IPSTR, IP2STR(&(results->addr->addr.u_addr.ip4)));
        ESP_LOGI("MDNS", "broker ip %s", address);

        return address;
    }

    std::optional<const char *> Mdns::FindHomeAssistant()
    {
        constexpr char HA_HOSTNAME[] = "homeassistant";
        esp_ip4_addr_t ip;
        esp_err_t err = mdns_query_a(HA_HOSTNAME, 1500, &ip);
        if (err != ESP_OK)
        {
            ESP_LOGW("MDNS", "couldn't find home assistant. err %d", err);
            // return eResult::ERROR_CONNECTION_FAILURE;
            return std::nullopt;
        }

        static char address[20]{};
        snprintf(address, 20, IPSTR, IP2STR(&ip));
        ESP_LOGI("MDNS", "home assistant ip %s", address);

        return address;
    }

} // namespace Baozi
