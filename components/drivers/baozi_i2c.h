#ifndef BAOZI_I2C_H__
#define BAOZI_I2C_H__

#include "driver/i2c.h"
#include "baozi_result.h"
#include <mutex>

namespace Baozi
{

    class I2C
    {
        static constexpr int I2C_MASTER_SCL_IO = 22;
        static constexpr int I2C_MASTER_SDA_IO = 21;
        static constexpr int I2C_MASTER_PORT = I2C_NUM_0;
        static constexpr int I2C_MASTER_FREQ_HZ = 100000;
        static constexpr int I2C_MASTER_TX_BUF_DISABLE = 0;
        static constexpr int I2C_MASTER_RX_BUF_DISABLE = 0;

    public:
        I2C(uint8_t deviceAddress);

        eResult Write(uint8_t regAddr, const uint8_t *data, uint16_t len);
        eResult Write(const uint8_t *data, uint16_t len);
        eResult Read(uint8_t regAddr, uint8_t *data, uint16_t len);
        eResult Read(uint8_t *data, uint16_t len);
        uint8_t printAddress() { return m_deviceAddress; }

    private:
        using Lock = std::lock_guard<std::mutex>;

        static bool s_isInitialized;
        static constexpr bool ACK_CHECK_EN = true;
        static constexpr bool ACK_CHECK_DIS = false;
        std::mutex m_mutex;

        I2C(const I2C &) = delete;
        I2C &operator=(const I2C &) = delete;

        static eResult init();
        eResult switchRegister(uint8_t regAddr);
        eResult read(uint8_t *data, uint16_t len);

        uint8_t m_deviceAddress;
    };

} // namespace Baozi

#endif
