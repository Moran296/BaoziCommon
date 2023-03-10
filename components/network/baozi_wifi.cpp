#include "baozi_wifi.h"
#include <cstring>
#include <cstdlib>
#include "baozi_log.h"

namespace Baozi
{

    using namespace Baozi::WifiFSM;

    void Wifi::eventHandler(void *event_handler_arg,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void *event_data)
    {
        BAO_LOG_INFO("WIFI event %ld", event_id);
        Wifi *wifi = reinterpret_cast<Wifi *>(event_handler_arg);

        if (event_base == WIFI_EVENT)
        {
            switch (event_id)
            {
            case WIFI_EVENT_STA_START:
                wifi->Dispatch(EVENT_StaStart{});
                break;

            case WIFI_EVENT_STA_CONNECTED:
                wifi->Dispatch(EVENT_StaConnected{});
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
            {
                wifi_event_sta_disconnected_t *e = (wifi_event_sta_disconnected_t *)event_data;
                wifi->Dispatch(EVENT_LoseConnection{.reason = (wifi_err_reason_t)e->reason});
                break;
            }
            case WIFI_EVENT_AP_START:
                BAO_LOG_INFO("AP started successfully\n");
                break;

            case WIFI_EVENT_AP_STOP:
                wifi->Dispatch(EVENT_APStop{});
                break;

            case WIFI_EVENT_AP_STACONNECTED:
                wifi->Dispatch(EVENT_UserConnected{});
                break;

            case WIFI_EVENT_AP_STADISCONNECTED:
            {
                wifi_event_sta_disconnected_t *e = (wifi_event_sta_disconnected_t *)event_data;
                wifi->Dispatch(EVENT_LoseConnection{.reason = (wifi_err_reason_t)e->reason});
                break;
            }
            }
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        {
            wifi->Dispatch(EVENT_GotIP{});
        }
    }

    Wifi::Wifi()
    {
    }

    void Wifi::Init()
    {
        init();
    }

    void Wifi::init()
    {
        esp_err_t err = 0;
        err = esp_netif_init();
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi init failed in line %d. err %d\n", __LINE__, err);
            configASSERT(false);
        }

        err = esp_event_loop_create_default();
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi init failed in line %d. err %d\n", __LINE__, err);
            configASSERT(false);
        }

        err = esp_event_handler_instance_register(ESP_EVENT_ANY_BASE,
                                                  ESP_EVENT_ANY_ID,
                                                  &eventHandler,
                                                  this,
                                                  NULL);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi init failed in line %d. err %d\n", __LINE__, err);
            configASSERT(false);
        }

        ap_netif = esp_netif_create_default_wifi_ap();
        configASSERT(ap_netif != nullptr);
        sta_netif = esp_netif_create_default_wifi_sta();
        configASSERT(sta_netif != nullptr);

        wifi_init_config_t initConf = WIFI_INIT_CONFIG_DEFAULT();
        initConf.nvs_enable = false;
        err = esp_wifi_init(&initConf);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi init failed in line %d. err %d\n", __LINE__, err);
            configASSERT(false);
        }

        Start();
    }

    bool Wifi::Connect(std::string_view ssid, std::string_view password)
    {
        configASSERT(IsRunning());

        if (ssid.empty() || password.empty())
        {
            return false;
        }

        if (IsInState<STATE_Connected>())
        {
            BAO_LOG_ERROR("already connected. call disconnect first\n");
            return false;
        }

        m_ssid = ssid;
        m_password = password;

        BAO_LOG_INFO("*connecting....*\n");
        Dispatch(EVENT_StaConnect{});
        return true;
    }

    bool Wifi::SwitchToAP()
    {
        configASSERT(IsRunning());

        if (IsInState<STATE_AP>())
        {
            BAO_LOG_WARNING("already AP\n");
            return false;
        }

        Dispatch(EVENT_APStart{});
        return true;
    }

    bool Wifi::Disconnect()
    {
        if (IsInState<STATE_Offline>())
        {
            BAO_LOG_WARNING("already offline\n");
            return false;
        }

        Dispatch(EVENT_Disconnect{});
        return true;
    }

    void Wifi::Reset()
    {
        m_ssid.clear();
        m_password.clear();

        Dispatch(EVENT_Disconnect{});
    }

    // ENTRY FUNCTIONS
    void Wifi::on_entry(STATE_Offline &)
    {
        BAO_LOG_INFO("wifi Offline\n");
    }

    void Wifi::on_entry(STATE_AP &)
    {
        BAO_LOG_INFO("WIFI IS AP\n");
    }

    void Wifi::on_entry(STATE_Connecting &state)
    {
        BAO_LOG_INFO("WIFI IS TRYING TO CONNECT......\n");
        sta_start();
    }

    void Wifi::on_entry(STATE_Connected &)
    {
        BAO_LOG_INFO("WIFI CONNECTED\n");
        m_retry_count = 0;
    }

    using return_state_t = std::optional<WifiFSM::States>;

    // ======================= Offline =======================
    return_state_t Wifi::on_event(STATE_Offline &, EVENT_APStart &)
    {
        BAO_LOG_INFO("state offline got AP Start event\n");
        ap_start();
        return STATE_AP{};
    }

    return_state_t Wifi::on_event(STATE_Offline &, EVENT_StaConnect &event)
    {
        BAO_LOG_INFO("state offline got StaConnect event\n");
        return STATE_Connecting{};
    }

    // ======================= AP =======================
    return_state_t Wifi::on_event(STATE_AP &, EVENT_APStart &)
    {
        BAO_LOG_INFO("state AP got APStart event\n");
        return STATE_AP{};
    }

    return_state_t Wifi::on_event(STATE_AP &, EVENT_APStop &)
    {
        BAO_LOG_INFO("state AP got APStop event\n");
        return STATE_Offline{};
    }

    return_state_t Wifi::on_event(STATE_AP &, EVENT_StaConnect &event)
    {
        BAO_LOG_INFO("state AP got StaConnect event\n");
        disconnect();
        return STATE_Connecting{};
    }

    return_state_t Wifi::on_event(STATE_AP &, EVENT_UserConnected &)
    {
        BAO_LOG_INFO("state AP got UserConnected event\n");
        return std::nullopt;
    }

    // ======================= Connecting =======================
    return_state_t Wifi::on_event(STATE_Connecting &, EVENT_LoseConnection &)
    {
        BAO_LOG_INFO("state Connecting got LoseConnection event\n");
        m_retry_count++;

        if (m_retry_count > MAX_RETRIES)
        {
            BAO_LOG_ERROR("WIFI RETRT COUNT EXCEEDED %d. RESETTING", MAX_RETRIES);
            esp_restart();
        }

        return std::nullopt;
    }

    return_state_t Wifi::on_event(STATE_Connecting &, EVENT_StaStart &)
    {
        BAO_LOG_INFO("state Connecting got StaStart event\n");
        sta_connect();
        return std::nullopt;
    }

    return_state_t Wifi::on_event(STATE_Connecting &, EVENT_StaConnected &)
    {
        BAO_LOG_INFO("state Connecting got connected event\n");
        return STATE_Connected{};
    }

    return_state_t Wifi::on_event(STATE_Connecting &, EVENT_GotIP &)
    {
        BAO_LOG_INFO("state Connecting got GotIP event\n");
        return STATE_Connected{};
    }

    // ======================= Connected =======================
    return_state_t Wifi::on_event(STATE_Connected &, EVENT_LoseConnection &event)
    {
        BAO_LOG_INFO("state Connected got LoseConnection event\n");

        if (event.reason == WIFI_REASON_ASSOC_LEAVE)
        {
            return STATE_Offline{};
        }

        return STATE_Connecting{};
    }

    return_state_t Wifi::on_event(STATE_Connected &, EVENT_Disconnect &)
    {
        BAO_LOG_INFO("state Connected got Disconnect event\n");
        disconnect();
        return STATE_Offline{};
    }

    return_state_t Wifi::on_event(STATE_Connected &, EVENT_GotIP &)
    {
        BAO_LOG_INFO("state Connected got GotIP event\n");
        return std::nullopt;
    }

    // Private functions

    bool Wifi::sta_start()
    {
        esp_err_t err = ESP_OK;
        err = esp_wifi_set_mode(WIFI_MODE_STA);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }
        wifi_config_t config;
        memset(&config, 0, sizeof(config));
        strlcpy((char *)config.sta.ssid, m_ssid.data(), sizeof(config.sta.ssid));
        strlcpy((char *)config.sta.password, m_password.data(), sizeof(config.sta.password));
        config.sta.bssid_set = false;
        config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;

        err = esp_wifi_set_config(WIFI_IF_STA, &config);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }

        err = esp_wifi_start();
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }

        return true;
    }

    bool Wifi::sta_connect()
    {
        BAO_LOG_INFO("sta connecting.....\n");
        esp_err_t err = esp_wifi_connect();
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }

        return true;
    }

    void Wifi::disconnect()
    {
        esp_wifi_disconnect();
        esp_wifi_stop();
    }

    bool Wifi::ap_start()
    {
        BAO_LOG_INFO("ap ssid %s", AP_SSID);

        esp_err_t err = ESP_OK;
        err = esp_wifi_set_mode(WIFI_MODE_AP);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }
        wifi_config_t config;
        memset(&config, 0, sizeof(config));
        strlcpy((char *)config.ap.ssid, AP_SSID, sizeof(config.ap.ssid));
        config.ap.max_connection = 4;
        config.ap.ssid_len = strlen(AP_SSID);
        config.ap.channel = 1;
        config.ap.authmode = WIFI_AUTH_OPEN;

        err = esp_wifi_set_config(WIFI_IF_AP, &config);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi ap start failed in line %d. err %d\n", __LINE__, err);
            return false;
        }

        BAO_LOG_INFO("starting ap.....\n");
        err = esp_wifi_start();
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("wifi sta connect failed in line %d. err %d\n", __LINE__, err);
            return false;
        }

        return true;
    }

    bool Wifi::IsConnected() const
    {
        return IsInState<WifiFSM::STATE_Connected>();
    }

} // Baozi