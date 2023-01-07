#ifndef _BAOZI_HA_COMMON_H__
#define _BAOZI_HA_COMMON_H__

#include "esp_mac.h"

namespace Baozi
{
    namespace HA
    {

        static inline constexpr const char *DEVICE_NAME = "BAOZI_TEST";
        static inline constexpr const char *SWITCH_ICON = "mdi:toggle-switch";

        inline std::string AddMacToName(const char *name)
        {
            static char mac[10]{0};
            static char macStr[10]{0};
            if (mac[0] == 0)
            {
                configASSERT(esp_efuse_mac_get_default((uint8_t *)mac) == ESP_OK);
                snprintf(macStr, sizeof(macStr), "%02X%02X%02X", mac[3], mac[4], mac[5]);
            }

            return std::string(name) + "_" + macStr;
        }

    } // HA
} // namespace Baozi

#endif