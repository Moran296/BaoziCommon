#include "baozi_pir.h"
#include "baozi_dht22.h"
#include "baozi_binary_sensor.h"
#include "baozi_device_manager.h"
#include "baozi_bh1750.h"
#include "baozi_presence.h"

using namespace Baozi;
constexpr auto PRINT_DELAY = 5_sec;

extern "C" void app_main(void)
{
    PirSensor pirSensor(26, HA::BinarySensor{"pir", "motion"});
    PresenceSensor presenceSensor(32, HA::BinarySensor{"presence", "occupancy"});
    DHTSensor dhtSensor(27,
                        HA::Sensor{HA::TEMPERATURE_SENSOR_CONFIG},
                        HA::Sensor{HA::HUMIDITY_SENSOR_CONFIG});
    BH1750 bh1750(HA::Sensor{HA::LIGHT_SENSOR_CONFIG});

    auto &manager = DeviceManager::GetInstance();
    manager.RegisterComponent((I_IndependentComponent *)&pirSensor);
    manager.RegisterComponent((I_PollingComponent *)&dhtSensor);
    manager.RegisterComponent((I_PollingComponent *)&bh1750);
    manager.RegisterComponent((I_PollingComponent *)&presenceSensor);

    manager.Run();
}
