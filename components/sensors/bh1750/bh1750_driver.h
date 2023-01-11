
#ifndef _BH1750_DRIVER_H__
#define _BH1750_DRIVER_H__

#include "baozi_i2c.h"
#include "baozi_result.h"

namespace Baozi
{
    class BH1750Driver
    {
    public:
        static constexpr uint8_t BH1750_I2C_ADDRESS_DEFAULT = 0x23;
        static constexpr uint8_t BH1750_I2C_ADDRESS_PULLUP = 0x5C;
        enum eMode
        {
            BH1750_CONTINUE_1LX_RES = 0x10,    /*!< Command to set measure mode as Continuously H-Resolution mode*/
            BH1750_CONTINUE_HALFLX_RES = 0x11, /*!< Command to set measure mode as Continuously H-Resolution mode2*/
            BH1750_CONTINUE_4LX_RES = 0x13,    /*!< Command to set measure mode as Continuously L-Resolution mode*/
            BH1750_ONETIME_1LX_RES = 0x20,     /*!< Command to set measure mode as One Time H-Resolution mode*/
            BH1750_ONETIME_HALFLX_RES = 0x21,  /*!< Command to set measure mode as One Time H-Resolution mode2*/
            BH1750_ONETIME_4LX_RES = 0x23,     /*!< Command to set measure mode as One Time L-Resolution mode*/
        };

        BH1750Driver(const uint16_t dev_addr = BH1750_I2C_ADDRESS_DEFAULT);

        BaoResult<float> GetData();

        eResult SetMode(eMode cmd_measure);
        eResult SetMeasureTime(uint8_t measure_time);
        eResult PowerDown();
        eResult PowerOn();

    private:
        I2C m_i2c;
    };

}

#endif