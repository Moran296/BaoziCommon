#ifndef BAOZI_DEBOUNCER_H__
#define BAOZI_DEBOUNCER_H__

#include "baozi_time_units.h"
#include "esp_timer.h"

namespace Baozi {

class Debouncer
{
public:
    Debouncer(MicroSeconds debounce_time = 1000) : debounce_time(debounce_time),
                                                   last_sample(0) {}
    inline bool IsValidNow()
    {
        if (esp_timer_get_time() - last_sample.value() > debounce_time.value())
        {
            last_sample = esp_timer_get_time();
            return true;
        }

        return false;
    }

private:
    const MicroSeconds debounce_time;
    MicroSeconds last_sample;
};

} // namespace Baozi

#endif