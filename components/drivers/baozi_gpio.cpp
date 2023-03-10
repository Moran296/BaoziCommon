#include "baozi_gpio.h"
#include "baozi_log.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Baozi
{

    GPIO_BASE::GPIO_BASE(int pin,
                         gpio_int_type_t isrType,
                         gpio_mode_t mode,
                         bool active_state,
                         bool pullup,
                         bool pulldown)
    {
        m_pin = gpio_num_t(pin);
        m_config.pin_bit_mask = 1ULL << pin;
        m_config.mode = mode;
        m_config.pull_down_en = pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
        m_config.pull_up_en = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
        m_config.intr_type = isrType;

        esp_err_t err = gpio_config(&m_config);
        if (err != ESP_OK)
        {
            BAO_LOG_ERROR("failed gpio config, err %d", err);
            configASSERT(false);
        }

        m_activeState = active_state;
    }

    /*==================
            GPO
    ===================*/
    GPO::GPO(int pin,
             bool active_state,
             bool pullup,
             bool pulldown) : GPIO_BASE(pin,
                                        GPIO_INTR_DISABLE,
                                        GPIO_MODE_OUTPUT,
                                        active_state,
                                        pullup,
                                        pulldown)
    {
        Set(false);
    }

    void GPO::Set(bool state)
    {
        uint32_t active = state ? m_activeState : !m_activeState;
        esp_err_t res = gpio_set_level(m_pin, active);
        configASSERT(res == ESP_OK);
        m_state = state;
    }

    void GPO::operator++(int)
    {
        Set(true);
    }
    void GPO::operator--(int)
    {
        Set(false);
    }

    GPO::operator bool() const
    {
        return m_state;
    }

    void GPO::Toggle()
    {
        Set(!m_state);
    }

    /*==================
            GPI
    ===================*/

    bool GPI::m_isr_driver_installed = false;

    GPI::GPI(int pin,
             gpio_int_type_t isrType,
             bool active_state,
             bool pullup,
             bool pulldown) : GPIO_BASE(pin,
                                        isrType,
                                        GPIO_MODE_INPUT,
                                        active_state,
                                        pullup,
                                        pulldown)
    {
    }

    GPI::operator bool() const
    {
        return IsActive();
    }
    bool GPI::IsActive() const
    {
        return 0 != gpio_get_level(m_pin);
    }

    bool GPI::IsHigh() const
    {
        return gpio_get_level(m_pin);
    }

    eResult GPI::RegisterISR(isr_func_t isrEventHandler, void *arg)
    {
        configASSERT(m_config.intr_type != GPIO_INTR_DISABLE);
        configASSERT(isrEventHandler != nullptr);

        esp_err_t res = ESP_OK;

        if (!m_isr_driver_installed)
        {
            res = gpio_install_isr_service(0);
            if (res != ESP_OK)
            {
                BAO_LOG_ERROR("set interrupt failed, err %d", res);
                return eResult::INVALID_STATE;
            }

            m_isr_driver_installed = true;
        }

        res = gpio_isr_handler_add(m_pin, isrEventHandler, arg);
        if (res != ESP_OK)
        {
            BAO_LOG_ERROR("set interrupt failed, err %d", res);
            return eResult::INVALID_STATE;
        }

        return eResult::SUCCESS;
    }

    eResult GPI::RemoveInterrupt()
    {
        esp_err_t res = gpio_isr_handler_remove(m_pin);
        if (res != ESP_OK)
        {
            BAO_LOG_ERROR("remove interrupt failed, err %d", res);
            return eResult::INVALID_STATE;
        }

        return eResult::SUCCESS;
    }

    void GPI::DisableInterrupt()
    {
        configASSERT(gpio_intr_disable(m_pin) == ESP_OK);
    }
    void GPI::EnableInterrupt()
    {
        configASSERT(gpio_intr_enable(m_pin) == ESP_OK);
    }

    eResult GPI::SetInterruptType(gpio_int_type_t type)
    {
        m_config.intr_type = type;
        esp_err_t res = gpio_set_intr_type(m_pin, type);
        return res == ESP_OK ? eResult::SUCCESS : eResult::INVALID_STATE;
    }

    //================= GPIO input with output =================

    GPIO_OD::GPIO_OD(int pin, bool pullup, bool pulldown) : GPIO_BASE(pin,
                                                                      GPIO_INTR_DISABLE,
                                                                      GPIO_MODE_INPUT_OUTPUT_OD,
                                                                      true,
                                                                      pullup,
                                                                      pulldown)
    {
        Set(!m_activeState);
    }

    bool GPIO_OD::IsActive() const
    {
        return 0 != gpio_get_level(m_pin);
    }
    bool GPIO_OD::IsHigh() const
    {
        return gpio_get_level(m_pin);
    }
    void GPIO_OD::Set(bool state)
    {
        esp_err_t res = gpio_set_level(m_pin, state);
        configASSERT(res == ESP_OK);
    }

} // namespace Baozi
