#include "dht_driver.h"

namespace Baozi::DHT
{

    Driver::Driver(int pin, int timeout) : m_pin(pin, true), m_timeout(timeout)
    {
        m_pin.Set(true);
    }

    eDHTResult Driver::Read(float &temperature, float &humidity);
    humidity = 0;
    temperature = 0;

    eDHTResult error_code = DHT_OK;
    int8_t i = 0;
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // this->pin_->digital_write(false);
    // this->pin_->pin_mode(gpio::FLAG_OUTPUT);
    // this->pin_->digital_write(false);
    m_pin.Set(false);

    ets_delay_us(m_timeout);
    // this->pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
    m_pin.Set(true);

    {
        InterruptLock lock;
        portDISABLE_INTERRUPTS();
        // Host pull up 20-40us then DHT response 80us
        // Start waiting for initial rising edge at the center when we
        // expect the DHT response (30us+40us)
        ets_delay_us(70);

        uint8_t bit = 7;
        uint8_t byte = 0;

        for (i = -1; i < 40; i++)
        {
            uint32_t start_time = micros();

            // Wait for rising edge
            while (!this->pin_->digital_read())
            {
                if (micros() - start_time > 90)
                {
                    if (i < 0)
                    {
                        error_code = 1;
                    }
                    else
                    {
                        error_code = 2;
                    }
                    break;
                }
            }
            if (error_code != 0)
                break;

            start_time = micros();
            uint32_t end_time = start_time;

            // Wait for falling edge
            while (this->pin_->digital_read())
            {
                if ((end_time = micros()) - start_time > 90)
                {
                    if (i < 0)
                    {
                        error_code = 3;
                    }
                    else
                    {
                        error_code = 4;
                    }
                    break;
                }
            }
            if (error_code != 0)
                break;

            if (i < 0)
                continue;

            if (end_time - start_time >= 40)
            {
                data[byte] |= 1 << bit;
            }
            if (bit == 0)
            {
                bit = 7;
                byte++;
            }
            else
                bit--;
        }

        portENABLE_INTERRUPTS();
    }

    if (!report_errors && error_code != 0)
        return false;

    switch (error_code)
    {
    case 1:
        ESP_LOGW(TAG, "Waiting for DHT communication to clear failed!");
        return false;
    case 2:
        ESP_LOGW(TAG, "Rising edge for bit %d failed!", i);
        return false;
    case 3:
        ESP_LOGW(TAG, "Requesting data from DHT failed!");
        return false;
    case 4:
        ESP_LOGW(TAG, "Falling edge for bit %d failed!", i);
        return false;
    case 0:
    default:
        break;
    }

    ESP_LOGVV(TAG,
              "Data: Hum=0b" BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN
              ", Temp=0b" BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN ", Checksum=0b" BYTE_TO_BINARY_PATTERN,
              BYTE_TO_BINARY(data[0]), BYTE_TO_BINARY(data[1]), BYTE_TO_BINARY(data[2]), BYTE_TO_BINARY(data[3]),
              BYTE_TO_BINARY(data[4]));

    uint8_t checksum_a = data[0] + data[1] + data[2] + data[3];
    // On the DHT11, two algorithms for the checksum seem to be used, either the one from the DHT22,
    // or just using bytes 0 and 2
    uint8_t checksum_b = this->model_ == DHT_MODEL_DHT11 ? (data[0] + data[2]) : checksum_a;

    if (checksum_a != data[4] && checksum_b != data[4])
    {
        if (report_errors)
        {
            ESP_LOGW(TAG, "Checksum invalid: %u!=%u", checksum_a, data[4]);
        }
        return false;
    }

    if (this->model_ == DHT_MODEL_DHT11)
    {
        if (checksum_a == data[4])
        {
            // Data format: 8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data + 8bit
            // check sum - some models always have 0 in the decimal part
            const uint16_t raw_temperature = uint16_t(data[2]) * 10 + (data[3] & 0x7F);
            *temperature = raw_temperature / 10.0f;
            if ((data[3] & 0x80) != 0)
            {
                // negative
                *temperature *= -1;
            }

            const uint16_t raw_humidity = uint16_t(data[0]) * 10 + data[1];
            *humidity = raw_humidity / 10.0f;
        }
        else
        {
            // For compatibility with DHT11 models which might only use 2 bytes checksums, only use the data from these two
            // bytes
            *temperature = data[2];
            *humidity = data[0];
        }
    }
    else
    {
        uint16_t raw_humidity = (uint16_t(data[0] & 0xFF) << 8) | (data[1] & 0xFF);
        uint16_t raw_temperature = (uint16_t(data[2] & 0xFF) << 8) | (data[3] & 0xFF);

        if (this->model_ != DHT_MODEL_DHT22_TYPE2 && (raw_temperature & 0x8000) != 0)
            raw_temperature = ~(raw_temperature & 0x7FFF);

        if (raw_temperature == 1 && raw_humidity == 10)
        {
            if (report_errors)
            {
                ESP_LOGW(TAG, "Invalid temperature+humidity! Sensor reported 1°C and 1%% Hum");
            }
            return false;
        }

        *humidity = raw_humidity * 0.1f;
        if (*humidity > 100)
            *humidity = NAN;
        *temperature = int16_t(raw_temperature) * 0.1f;
    }

    if (*temperature == 0.0f && (*humidity == 1.0f || *humidity == 2.0f))
    {
        if (report_errors)
        {
            ESP_LOGW(TAG, "DHT reports invalid data. Is the update interval too high or the sensor damaged?");
        }
        return false;
    }

    return true;
}

} // namespace Baozi::DHT
