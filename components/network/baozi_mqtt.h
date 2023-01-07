#ifndef BAOZI_MQTT_H__
#define BAOZI_MQTT_H__

#include "mqtt_client.h"
#include <functional>
#include <mutex>
#include <map>
#include <queue>
#include <memory>
#include "baozi_json.h"
#include "fsm_taskless.h"
#include "baozi_result.h"
#include "baozi_nvs.h"

namespace Baozi
{

    using mqtt_handler_callback = std::function<void(const std::string &topic, const BaoJson &payload)>;

    namespace MqttFSM
    {

        struct STATE_DISABLED
        {
            static constexpr const char *NAME = "STATE_DISABLED";
        };

        struct STATE_CONNECTING
        {
            static constexpr const char *NAME = "STATE_CONNECTING";
        };

        struct STATE_CONNECTED
        {
            static constexpr const char *NAME = "STATE_CONNECTED";
        };

        using States = std::variant<STATE_DISABLED,
                                    STATE_CONNECTING,
                                    STATE_CONNECTED>;

        struct EVENT_BEFORE_CONNECT
        {
            static constexpr const char *NAME = "EVENT_BEFORE_CONNECT";
        };
        struct EVENT_CONNECTED
        {
            static constexpr const char *NAME = "EVENT_CONNECTED";
        };
        struct EVENT_CONNECT
        {
            static constexpr const char *NAME = "EVENT_CONNECT";
        };
        struct EVENT_DISCONNECTED
        {
            static constexpr const char *NAME = "EVENT_DISCONNECTED";
        };
        struct EVENT_SUBSCRIBE
        {
            static constexpr const char *NAME = "EVENT_SUBSCRIBE";
        };
        struct EVENT_SUBSCRIBED
        {
            static constexpr const char *NAME = "EVENT_SUBSCRIBED";
        };
        struct EVENT_PUBLISHED
        {
            static constexpr const char *NAME = "EVENT_PUBLISHED";
        };
        struct EVENT_ERROR
        {
            static constexpr const char *NAME = "EVENT_ERROR";
        };
        struct EVENT_INCOMING_DATA
        {
            static constexpr const char *NAME = "EVENT_INCOMING_DATA";
            std::string topic;
            BaoJson payload;
        };

        using Events = std::variant<EVENT_BEFORE_CONNECT,
                                    EVENT_CONNECTED,
                                    EVENT_CONNECT,
                                    EVENT_DISCONNECTED,
                                    EVENT_SUBSCRIBE,
                                    EVENT_SUBSCRIBED,
                                    EVENT_PUBLISHED,
                                    EVENT_ERROR,
                                    EVENT_INCOMING_DATA>;

    } // namespace MqttFSM

    class MqttClient : public FsmTaskless<MqttClient, MqttFSM::States, MqttFSM::Events>
    {
    public:
        struct Config
        {
            const char *broker_ip;
            const char *username;
            const char *password;
            const char *clientId; // leave empty to use mac address
            int port = 1883;
            std::pair<const char *, const char *> willTopicAndPayload;
            std::function<void()> onConnectCallback;
        };

        MqttClient();
        void On(const char *topic, mqtt_handler_callback callback);
        eResult Publish(const char *topic, const BaoJson &msg);
        eResult Publish(const char *topic, const char *msg);
        eResult TryConnect(const Config &config);
        bool IsConnected() const;

        void on_entry(MqttFSM::STATE_DISABLED &);
        void on_entry(MqttFSM::STATE_CONNECTING &);
        void on_entry(MqttFSM::STATE_CONNECTED &);

        using return_state_t = std::optional<MqttFSM::States>;

        return_state_t on_event(MqttFSM::STATE_DISABLED &, MqttFSM::EVENT_CONNECT &);
        return_state_t on_event(MqttFSM::STATE_DISABLED &, MqttFSM::EVENT_BEFORE_CONNECT &);

        return_state_t on_event(MqttFSM::STATE_CONNECTING &, MqttFSM::EVENT_CONNECTED &);
        return_state_t on_event(MqttFSM::STATE_CONNECTING &, MqttFSM::EVENT_DISCONNECTED &);
        return_state_t on_event(MqttFSM::STATE_CONNECTING &, MqttFSM::EVENT_ERROR &);

        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_SUBSCRIBE &);
        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_SUBSCRIBED &);
        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_PUBLISHED &);
        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_ERROR &);
        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_DISCONNECTED &);
        return_state_t on_event(MqttFSM::STATE_CONNECTED &, MqttFSM::EVENT_INCOMING_DATA &);

        template <typename State, typename Event>
        auto on_event(State &state, Event &event)
        {
            printf("unhandled event!: %s got %s\n", state.NAME, event.NAME);
            return std::nullopt;
        }

    private:
        struct mqtt_event_handler_t
        {
            mqtt_handler_callback cb;
            bool isSubscribed{};
        };
        using handlers_t = std::map<std::string, mqtt_event_handler_t>;

        esp_mqtt_client_handle_t m_client{};
        handlers_t m_handlers;
        std::mutex m_mutex; // protects m_handlers... consider not using at all...
        std::function<void()> m_onConnectCallback;

        bool connect(const Config &config);
        bool subscribe(const char *topic);
        eResult publish(const char *topic, const char *payload);
        void reSubscribeHandlers();

        static void mqttEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    };

} // namespace Baozi

#endif