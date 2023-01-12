#ifndef _BAOZI_DEVICE_MANAGER_H__
#define _BAOZI_DEVICE_MANAGER_H__

#include "baozi_connectivity_manager.h"
#include "baozi_component.h"
#include "baozi_time_units.h"
#include "baozi_fota.h"

namespace Baozi
{
    class DeviceManager
    {
    public:
        static constexpr MilliSeconds POLLING_INTERVAL = 2500;
        static constexpr Seconds WAIT_FOR_CONNECTION_INTERVAL = 5;

        static DeviceManager &GetInstance();

        void Run();
        MqttClient &GetMqttClient();

        void RegisterComponent(I_IndependentComponent *component);
        void RegisterComponent(I_PollingComponent *component);

    private:
        DeviceManager() = default;
        void loop();
        void wait_for_connection();
        void register_components();

        bool m_isRunning = false;
        ConnectivityManager m_connectivityManager;
        std::vector<I_IndependentComponent *> m_independentComponents;
        std::vector<I_PollingComponent *> m_pollingComponents;
        FotaHandler m_fotaHandler;
    };
}

#endif