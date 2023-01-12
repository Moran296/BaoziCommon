#ifndef _BAOZI_MDNS_H__
#define _BAOZI_MDNS_H__

#include <optional>

namespace Baozi
{

    class Mdns
    {
    public:
        static bool Init(const char*);
        static void AdvertiseMqtt();
        static void AdvertiseESPNOW();
        static void AdvertiseBaozi();
        static std::optional<const char *> FindBroker();
        static std::optional<const char *> FindHomeAssistant();
    };

} // namespace Baozi

#endif
