#include "baozi_pir.h"
#include "baozi_binary_sensor.h"
#include "baozi_device_manager.h"

using namespace Baozi;
constexpr auto PRINT_DELAY = 5_sec;

extern "C" void app_main(void)
{
    PirSensor pirSensor(27, HA::BinarySensor{"pir", "motion"});

    auto &manager = DeviceManager::GetInstance();
    manager.RegisterComponent((I_IndependentComponent *)&pirSensor);

    manager.Run();
}
