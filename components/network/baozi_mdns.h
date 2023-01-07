#ifndef _BAOZI_MDNS_H__
#define _BAOZI_MDNS_H__

#include <optional>

namespace Baozi
{

    class Mdns
    {
    public:
        static bool Init();
        static void AdvertiseMqtt();
        static void AdvertiseESPNOW();
        static std::optional<const char *> FindBroker();
        static std::optional<const char *> FindHomeAssistant();
    };

} // namespace Baozi

#endif
