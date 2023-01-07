#ifndef _BAOZI_SENSOR_INTERFACE_H__
#define _BAOZI_SENSOR_INTERFACE_H__

namespace Baozi
{

    class I_IndependentComponent
    {
    public:
        virtual eResult Register() = 0;
        virtual ~I_IndependentComponent() = default;
    };

    class I_PollingComponent
    {
    public:
        virtual void Poll() = 0;
        virtual eResult Register() = 0;
        virtual ~I_PollingComponent() = default;
    };

} // namespace Baozi

#endif