/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "bh1750_driver.h"
#include "baozi_log.h"

namespace Baozi
{

    constexpr float BH_1750_MEASUREMENT_ACCURACY = 1.2; /*!< the typical measurement accuracy of  BH1750 sensor */
    constexpr uint8_t BH1750_POWER_DOWN = 0x00;         /*!< Command to set Power Down*/
    constexpr uint8_t BH1750_POWER_ON = 0x01;           /*!< Command to set Power On*/

    BH1750Driver::BH1750Driver(const uint16_t dev_addr) : m_i2c(dev_addr)
    {
    }

    eResult BH1750Driver::PowerDown()
    {
        return m_i2c.Write(&BH1750_POWER_DOWN, 1);
    }

    eResult BH1750Driver::PowerOn()
    {
        return m_i2c.Write(&BH1750_POWER_ON, 1);
    }

    eResult BH1750Driver::SetMeasureTime(uint8_t measure_time)
    {
        uint32_t i = 0;
        uint8_t buf[2] = {0x40, 0x60}; // constant part of the the MTreg
        buf[0] |= measure_time >> 5;
        buf[1] |= measure_time & 0x1F;
        for (i = 0; i < 2; i++)
        {
            eResult ret = m_i2c.Write(&buf[i], 1);
            if (eResult::SUCCESS != ret)
            {
                return ret;
            }
        }
        return eResult::SUCCESS;
    }

    eResult BH1750Driver::SetMode(BH1750Driver::eMode cmd_measure)
    {
        uint8_t cmd = cmd_measure;
        return m_i2c.Write(&cmd, 1);
    }

    BaoResult<float> BH1750Driver::GetData()
    {
        uint16_t bh1750_data = 0;
        uint8_t buf[2] = {0};

        if (eResult res = m_i2c.Read((uint8_t *)&buf, 2); eResult::SUCCESS != res)
        {
            BAO_LOG_ERROR("bh1750 read data failed");
            return BaoResult<float>::Error(res);
        }

        bh1750_data = (buf[0] << 8) | buf[1];
        float data_lux = bh1750_data / BH_1750_MEASUREMENT_ACCURACY;
        return BaoResult<float>::Ok(data_lux);
    }

}