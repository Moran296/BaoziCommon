#include "baozi_pir.h"
#include "baozi_time_units.h"

namespace Baozi
{

    PirSensor::PirSensor(int pin, HA::BinarySensor &&sensor) : m_gpi(pin, GPIO_INTR_POSEDGE, true, false, true),
                                                               m_sensor(std::move(sensor)),
                                                               m_debouncer(MicroSeconds(Seconds(1)))
    {
        configASSERT(xTaskCreatePinnedToCore(taskHandler, "PirSensor", 2048, this, 5, &m_task, 0) == pdPASS);
        configASSERT(m_gpi.RegisterISR(isrHandler, this) == eResult::SUCCESS);
        m_gpi.EnableInterrupt();
    }

    void PirSensor::isrHandler(void *arg)
    {
        PirSensor *pir = static_cast<PirSensor *>(arg);
        if (not pir->m_debouncer.IsValidNow())
        {
            return;
        }

        BaseType_t pxHigherPriorityTaskWoken;
        vTaskNotifyGiveFromISR(pir->m_task, &pxHigherPriorityTaskWoken);

        if (pxHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }

    void PirSensor::taskHandler(void *arg)
    {
        PirSensor *pir = static_cast<PirSensor *>(arg);
        pir->loop();
    }

    void PirSensor::loop()
    {
        bool lastState = false;
        for (;;)
        {
            uint32_t gotEvent = ulTaskNotifyTake(pdTRUE, Seconds(5).toTicks());
            if (gotEvent > 0 && not lastState)
            {
                m_sensor.Publish(true);
                lastState = true;
            }
            else if (lastState)
            {
                m_sensor.Publish(false);
                lastState = false;
            }
        }
    }

} // namespace Baozi