#ifndef BAOZI_WIFI__
#define BAOZI_WIFI__

#include "esp_netif.h"
#include "esp_event.h"
#include "fsm_taskless.h"
#include "esp_wifi.h"
#include <variant>
#include <string_view>
#include <string>

namespace Baozi
{

    namespace WifiFSM
    {

        struct STATE_Offline
        {
        };
        struct STATE_AP
        {
        };
        struct STATE_Connecting
        {
        };
        struct STATE_Connected
        {
        };
        using States = std::variant<STATE_Offline, STATE_AP, STATE_Connected, STATE_Connecting>;

        struct EVENT_APStart
        {
        };
        struct EVENT_APStop
        {
        };
        struct EVENT_Disconnect
        {
        };
        struct EVENT_StaConnect
        {
        };
        struct EVENT_StaStart
        {
        };
        struct EVENT_StaConnected
        {
        };
        struct EVENT_LoseConnection
        {
            wifi_err_reason_t reason;
        };
        struct EVENT_UserConnected
        {
        };
        struct EVENT_GotIP
        {
        };
        using Events = std::variant<EVENT_APStart, EVENT_APStop, EVENT_Disconnect, EVENT_StaConnect, EVENT_StaStart, EVENT_StaConnected, EVENT_LoseConnection, EVENT_UserConnected, EVENT_GotIP>;

    } // namespace WifiFSM

    class Wifi : public FsmTaskless<Wifi, WifiFSM::States, WifiFSM::Events>
    {
        static constexpr int MAX_RETRIES = 15;

    public:
        Wifi();
        void Init();
        bool Connect(std::string_view ssid, std::string_view password);
        bool SwitchToAP();
        bool Disconnect();
        bool IsConnected() const;
        void Reset();

        void on_entry(WifiFSM::STATE_Offline &);
        void on_entry(WifiFSM::STATE_AP &);
        void on_entry(WifiFSM::STATE_Connecting &);
        void on_entry(WifiFSM::STATE_Connected &);

        using return_state_t = std::optional<WifiFSM::States>;

        // Offline
        return_state_t on_event(WifiFSM::STATE_Offline &, WifiFSM::EVENT_APStart &);
        return_state_t on_event(WifiFSM::STATE_Offline &, WifiFSM::EVENT_StaConnect &);
        // AP
        return_state_t on_event(WifiFSM::STATE_AP &, WifiFSM::EVENT_APStart &);
        return_state_t on_event(WifiFSM::STATE_AP &, WifiFSM::EVENT_APStop &);
        return_state_t on_event(WifiFSM::STATE_AP &, WifiFSM::EVENT_StaConnect &);
        return_state_t on_event(WifiFSM::STATE_AP &, WifiFSM::EVENT_UserConnected &);
        // COnnecting
        return_state_t on_event(WifiFSM::STATE_Connecting &, WifiFSM::EVENT_LoseConnection &);
        return_state_t on_event(WifiFSM::STATE_Connecting &, WifiFSM::EVENT_StaStart &);
        return_state_t on_event(WifiFSM::STATE_Connecting &, WifiFSM::EVENT_StaConnected &);
        return_state_t on_event(WifiFSM::STATE_Connecting &, WifiFSM::EVENT_GotIP &);
        // Connected
        return_state_t on_event(WifiFSM::STATE_Connected &, WifiFSM::EVENT_LoseConnection &);
        return_state_t on_event(WifiFSM::STATE_Connected &, WifiFSM::EVENT_GotIP &);
        return_state_t on_event(WifiFSM::STATE_Connected &, WifiFSM::EVENT_Disconnect &);
        // Default
        template <typename State, typename Event>
        return_state_t on_event(State &, Event &)
        {
            printf("default event on wifi event handler\n");
            return std::nullopt;
        }

    private:
        bool ap_start();
        void disconnect();
        bool sta_start();
        bool sta_connect();
        void init();

        static constexpr char AP_SSID[] = "BAOZI_AP"; // TODO: make this configurable

        std::string m_ssid{};
        std::string m_password{};
        int m_retry_count = 0;
        esp_netif_t *ap_netif{};
        esp_netif_t *sta_netif{};

        static void eventHandler(void *event_handler_arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void *event_data);
    };

} // Baozi

#endif
