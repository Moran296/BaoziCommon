#ifndef _BAOZI_TRY_H__
#define _BAOZI_TRY_H__

#include "baozi_time_units.h"

namespace Baozi
{

    template <typename Func>
    class BaoTry
    {
    public:
        BaoTry(Func &&func) : m_func(std::forward<Func>(func)) {}
        BaoTry &Times(uint8_t times)
        {
            m_times = times;
            return *this;
        }

        BaoTry &Delay(TimeUnit auto delay)
        {
            m_delay = delay.toTicks();
            return *this;
        }

        template <typename... Params>
        bool Run(Params &&...params)
        {
            for (;;)
            {
                if (m_times == 0)
                {
                    return false;
                }

                if (m_func(std::forward<Params>(params)...))
                {
                    return true;
                }

                m_times--;
                vTaskDelay(m_delay);
            }
        }

    private:
        uint8_t m_times{1};
        TickType_t m_delay{0};
        Func m_func;
    };

}

#endif