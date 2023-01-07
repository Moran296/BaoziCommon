#ifndef BAOZI_PIR_H__
#define BAOZI_PIR_H__

#include "baozi_binary_sensor.h"
#include "baozi_log.h"
#include "baozi_gpio.h"
#include "util_debouncer.h"
#include "baozi_result.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "baozi_component.h"

namespace Baozi
{

    class PirSensor : public I_IndependentComponent
    {
    public:
        PirSensor(int pin, HA::BinarySensor &&sensor);
        eResult Register() override { return m_sensor.Register(); }

    private:
        GPI m_gpi;
        HA::BinarySensor m_sensor;
        Debouncer m_debouncer;
        TaskHandle_t m_task = nullptr;

        void loop();

        static void isrHandler(void *arg);
        static void taskHandler(void *arg);
    };

} // namespace Baozi

#endif
