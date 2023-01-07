#include "baozi_mqtt.h"
#include "baozi_log.h"
#include "baozi_mdns.h"

namespace Baozi
{

    using namespace MqttFSM;

    //===============================EVENT HANDLER ==================================================

    void MqttClient::mqttEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        MqttClient *client = reinterpret_cast<MqttClient *>(arg);

        esp_mqtt_event_handle_t event = reinterpret_cast<esp_mqtt_event_handle_t>(event_data);
        switch ((esp_mqtt_event_id_t)event_id)
        {
        case MQTT_EVENT_BEFORE_CONNECT:
            client->Dispatch(EVENT_BEFORE_CONNECT{});
            break;
        case MQTT_EVENT_CONNECTED:
            client->Dispatch(EVENT_CONNECTED{});
            break;
        case MQTT_EVENT_DISCONNECTED:
            client->Dispatch(EVENT_DISCONNECTED{});
            break;
        case MQTT_EVENT_SUBSCRIBED:
            BAO_LOG_INFO("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            client->Dispatch(EVENT_SUBSCRIBED{});
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            BAO_LOG_INFO("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            client->Dispatch(EVENT_PUBLISHED{});
            break;
        case MQTT_EVENT_DATA:
            client->Dispatch(EVENT_INCOMING_DATA{.topic = event->topic, .payload = BaoJson::Parse(event->data).value_or(BaoJson{})});
            break;
        case MQTT_EVENT_ERROR:
            client->Dispatch(EVENT_ERROR{});
            break;
        default:
            configASSERT(false);
            break;
        }
    }

    //===============================================================================================
    //===============================PUBLIC METHODS ==================================================
    //===============================================================================================

    MqttClient::MqttClient()
    {
        Start(STATE_DISABLED{});
    }

    void MqttClient::On(const char *topic, mqtt_handler_callback callback)
    {
        configASSERT(callback);

        std::lock_guard<std::mutex> lock(m_mutex);
        mqtt_event_handler_t handler{.cb = callback, .isSubscribed = subscribe(topic)};
        m_handlers.insert_or_assign(std::string{topic}, std::move(handler));
    }

    eResult MqttClient::Publish(const char *topic, const BaoJson &msg)
    {
        auto payload = msg.PrintRaw();
        if (payload == nullptr)
        {
            BAO_LOG_ERROR("could not print json");
            return eResult::INVALID_STATE;
        }

        BAO_LOG_INFO("publishing json to %s", topic);
        return publish(topic, payload.get());
    }

    eResult MqttClient::Publish(const char *topic, const char *msg)
    {
        configASSERT(msg != nullptr);

        BAO_LOG_INFO("publishing string to %s", topic);
        return publish(topic, msg);
    }

    eResult MqttClient::TryConnect(const MqttClient::Config &config)
    {
        if (IsInState<STATE_CONNECTED>())
        {
            BAO_LOG_INFO("mqtt already connected");
            return eResult::SUCCESS;
        }

        if (!config.broker_ip || strlen(config.broker_ip) < 8)
        {
            BAO_LOG_ERROR("mqtt ip not set");
            return eResult::INVALID_STATE;
        }

        m_onConnectCallback = config.onConnectCallback;
        bool success = connect(config);
        if (!success)
        {
            BAO_LOG_ERROR("mqtt connect failed");
            return eResult::INVALID_STATE;
        }

        Dispatch(EVENT_CONNECT{});
        return eResult::SUCCESS;
    }

    //================================ENTRY FUNCTIONS ================================================

    void MqttClient::on_entry(STATE_DISABLED &state)
    {
        BAO_LOG_INFO("entered %s", state.NAME);
    }

    void MqttClient::on_entry(STATE_CONNECTING &state)
    {
        BAO_LOG_INFO("entered %s", state.NAME);
    }

    void MqttClient::on_entry(STATE_CONNECTED &state)
    {
        BAO_LOG_INFO("entered %s", state.NAME);
        reSubscribeHandlers();

        if (m_onConnectCallback)
            m_onConnectCallback();
    }

    //===============================================================================================
    //================================ STATE MACHINE ================================================
    //===============================================================================================

    using return_state_t = MqttClient::return_state_t;

    // STATE_DISABLED

    return_state_t MqttClient::on_event(STATE_DISABLED &state, EVENT_CONNECT &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return STATE_CONNECTING{};
    }

    return_state_t MqttClient::on_event(STATE_DISABLED &state, EVENT_BEFORE_CONNECT &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return STATE_CONNECTING{};
    }

    // STATE_DISABLED

    return_state_t MqttClient::on_event(STATE_CONNECTING &state, EVENT_CONNECTED &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return STATE_CONNECTED{};
    }

    return_state_t MqttClient::on_event(STATE_CONNECTING &state, EVENT_DISCONNECTED &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return std::nullopt;
    }

    return_state_t MqttClient::on_event(STATE_CONNECTING &state, EVENT_ERROR &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return std::nullopt;
    }

    // STATE_CONNECTED

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_SUBSCRIBE &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return std::nullopt;
    }

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_SUBSCRIBED &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return std::nullopt;
    }

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_PUBLISHED &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return std::nullopt;
    }

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_ERROR &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return STATE_CONNECTING{};
    }

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_DISCONNECTED &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);

        return STATE_CONNECTING{};
    }

    return_state_t MqttClient::on_event(STATE_CONNECTED &state, EVENT_INCOMING_DATA &event)
    {
        BAO_LOG_DEBUG("%s got %s", state.NAME, event.NAME);
        configASSERT(event.payload);

        std::lock_guard<std::mutex> lock(m_mutex);

        bool found = false;
        std::for_each(m_handlers.begin(), m_handlers.end(), [&event, &found](auto &handler)
                      {
        bool match = event.topic == handler.first ||
            (handler.first.ends_with("#") && event.topic.starts_with(handler.first.substr(0, handler.first.size() - 1)));

        if (!match)
            return;

        BAO_LOG_INFO("calling handler for topic %s", event.topic.c_str());
        handler.second.cb(event.topic, event.payload);
        found = true; });

        return std::nullopt;
    }

    //===============================================================================================

    eResult MqttClient::publish(const char *topic, const char *payload)
    {
        int err = esp_mqtt_client_publish(m_client, topic, payload, strlen(payload), 0, false);
        if (err == -1)
        {
            BAO_LOG_ERROR("mqtt publish failed");
            return eResult::FAIL;
        }

        return eResult::SUCCESS;
    }

    bool MqttClient::connect(const MqttClient::Config &baozi_config)
    {
        esp_mqtt_client_config_t esp_config{};
        char mqttHost[50]{};

        if (m_client != nullptr)
        {
            return false;
        }

        snprintf(mqttHost, sizeof(mqttHost), "mqtt://%s:%d", baozi_config.broker_ip, baozi_config.port);
        BAO_LOG_INFO("trying to connect to %s", mqttHost);

        esp_config.broker.address.uri = mqttHost;
        esp_config.broker.address.hostname = mqttHost;
        esp_config.credentials.client_id = baozi_config.clientId;
        esp_config.credentials.set_null_client_id = baozi_config.clientId == nullptr;
        esp_config.credentials.username = baozi_config.username;
        esp_config.credentials.authentication.password = baozi_config.password;
        esp_config.session.last_will.topic = baozi_config.willTopicAndPayload.first;
        esp_config.session.last_will.msg = baozi_config.willTopicAndPayload.second;
        esp_config.broker.address.port = baozi_config.port;

        m_client = esp_mqtt_client_init(&esp_config);
        if (!m_client)
        {
            BAO_LOG_ERROR("could not create mqtt client");
            return false;
        }

        esp_err_t err = esp_mqtt_client_register_event(m_client, MQTT_EVENT_ANY, mqttEventHandler, this);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("could not register event handler");
            return false;
        }

        err = esp_mqtt_client_start(m_client);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("could not start mqtt");
            return false;
        }

        BAO_LOG_INFO("mqtt initiated");
        return true;
    }

    bool MqttClient::subscribe(const char *topic)
    {
        if (not IsInState<STATE_CONNECTED>())
        {
            BAO_LOG_WARNING("cannot subscribe, not connected");
            return false;
        }

        esp_err_t err = esp_mqtt_client_subscribe(m_client, topic, 0);
        if (err == -1)
        {
            BAO_LOG_ERROR("subscribe failed, err %d", err);
            return false;
        }

        return true;
    }

    void MqttClient::reSubscribeHandlers()
    {
        configASSERT(IsInState<STATE_CONNECTED>());

        std::for_each(m_handlers.begin(), m_handlers.end(), [this](auto &handler)
                      { handler.second.isSubscribed = subscribe(handler.first.c_str()); });
    }

    bool MqttClient::IsConnected() const
    {
        return IsInState<STATE_CONNECTED>();
    }

} // namespace Baozi
