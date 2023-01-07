#include "baozi_device_manager.h"

namespace Baozi
{

    DeviceManager &DeviceManager::GetInstance()
    {
        static DeviceManager instance;
        return instance;
    }

    void DeviceManager::Run()
    {
        m_connectivityManager.Init();
        wait_for_connection();
        register_components();

        m_isRunning = true;

        loop();
    }

    void DeviceManager::loop()
    {
        for (;;)
        {
            BaoDelay(POLLING_INTERVAL);

            for (auto component : m_pollingComponents)
                component->Poll();

            if (not m_connectivityManager.IsConnected())
            {
                wait_for_connection();
                register_components();
            }
        }
    }

    MqttClient &DeviceManager::GetMqttClient()
    {
        return m_connectivityManager.GetMqttClient();
    }

    void DeviceManager::RegisterComponent(I_IndependentComponent *component)
    {
        m_independentComponents.push_back(component);
    }

    void DeviceManager::RegisterComponent(I_PollingComponent *component)
    {
        m_pollingComponents.push_back(component);
    }

    void DeviceManager::wait_for_connection()
    {
        while (not m_connectivityManager.IsConnected())
            BaoDelay(WAIT_FOR_CONNECTION_INTERVAL);
    }

    void DeviceManager::register_components()
    {
        for (auto component : m_independentComponents)
            component->Register();

        for (auto component : m_pollingComponents)
            component->Register();
    }

}
