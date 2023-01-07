#ifndef BAOZI_NVS__
#define BAOZI_NVS__

#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "baozi_result.h"
#include <mutex>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include "baozi_log.h"

namespace Baozi {

class NVS
{
    static char s_stringBuffer[256];
public:
    NVS(const char *nvs_namespace);

    template <typename T>
    BaoResult<T> Get(const char *key);

    template <typename T>
    eResult Set(const char *key, const T& value, bool shouldCommit = true);

    eResult GetString(const char *key, char *value, size_t maxLen);
    eResult SetString(const char *key, const char *value);
    eResult SetBlob(const char *key, const void *value, size_t len);
    eResult GetBlob(const char *key, void *value, size_t maxLen, size_t &actualLen);
    eResult Erase(const char *key);
    eResult Erase();
    eResult Commit();

private:
    static bool s_isInitialized;
    static eResult init();

    nvs_handle_t m_handle;
    std::mutex m_mutex;

    NVS(const NVS &) = delete;
    NVS &operator=(const NVS &) = delete;

    template <std::integral T>
    BaoResult<T> getNumber(const char *key);
    template <std::integral T>
    eResult setNumber(const char *key, T value);

};

template <typename T>
BaoResult<T> NVS::Get(const char *key)
{
    if constexpr(std::is_integral_v<T>) {
        return getNumber<T>(key);
    }

    if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, const char *>) {
        eResult res = GetString(key, s_stringBuffer, sizeof(s_stringBuffer));
        if (res == eResult::SUCCESS)
            return BaoResult<T>::Ok(s_stringBuffer);
        else
            return BaoResult<T>::Error(res);
    }
    else {
        if constexpr (not std::is_trivially_constructible_v<T> || not std::is_trivially_copyable_v<T>)
            static_assert(always_false<T>, "NVS::Get() for blobs can only be used with trivially constructible and copyable types. You can use GetBlob instead");

        T value;
        size_t actual_len{0};
        eResult res = GetBlob(key, &value, sizeof(value), actual_len);
        if (res == eResult::SUCCESS)
            return BaoResult<T>::Ok(std::move(value));
        else
            return BaoResult<T>::Error(res);
    }
}

template <typename T>
eResult NVS::Set(const char *key, const T& value, bool shouldCommit)
{
    eResult res{eResult::SUCCESS};
    if constexpr(std::is_integral_v<T>)
        res = setNumber<T>(key, value);
    else if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
        res = SetString(key, value.c_str());
    else if constexpr(std::is_same_v<T, const char *>)
        res = SetString(key, value);
    else
        res = SetBlob(key, &value, sizeof(T));

    if (res == eResult::SUCCESS && shouldCommit)
        res = Commit();

    return res;
}

template <std::integral T>
BaoResult<T> NVS::getNumber(const char *key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    T value{0};

    esp_err_t err{ESP_OK};
    if constexpr(std::is_same_v<T, uint8_t> || std::is_same_v<T, bool>)
        err = nvs_get_u8(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, int8_t> || std::is_same_v<T, char>)
        err = nvs_get_i8(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, uint16_t> || std::is_same_v<T, unsigned short>)
        err = nvs_get_u16(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, int16_t> || std::is_same_v<T, short>)
        err = nvs_get_i16(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, uint32_t> || std::is_same_v<T, unsigned int>)
        err = nvs_get_u32(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, int32_t> || std::is_same_v<T, int>)
        err = nvs_get_i32(m_handle, key, (int32_t*)&value);
    else if constexpr(std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long>)
        err = nvs_get_u64(m_handle, key, &value);
    else if constexpr(std::is_same_v<T, int64_t> || std::is_same_v<T, long long>)
        err = nvs_get_i64(m_handle, key, &value);
    else {
        static_assert(always_false<T>, "Unsupported type");
    }


    if (err == ESP_OK)
        return BaoResult<T>::Ok(value);
    else if (err == ESP_ERR_NVS_NOT_FOUND)
        return BaoResult<T>::Error(eResult::NOT_FOUND);
    else {
        BAO_LOG_ERROR("Error reading %s: %s", key, esp_err_to_name(err));
        return BaoResult<T>::Error(eResult::FLASH_FAILURE);
    }
}

template <std::integral T>
eResult NVS::setNumber(const char *key, T value) {
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err{ESP_OK};
    if constexpr(std::is_same_v<T, uint8_t> || std::is_same_v<T, bool> || std::is_same_v<T, unsigned char>)
        err = nvs_set_u8(m_handle, key, value);
    else if constexpr(std::is_same_v<T, int8_t> || std::is_same_v<T, char>)
        err = nvs_set_i8(m_handle, key, value);
    else if constexpr(std::is_same_v<T, uint16_t> || std::is_same_v<T, unsigned short>)
        err = nvs_set_u16(m_handle, key, value);
    else if constexpr(std::is_same_v<T, int16_t> || std::is_same_v<T, short>)
        err = nvs_set_i16(m_handle, key, value);
    else if constexpr(std::is_same_v<T, uint32_t> || std::is_same_v<T, unsigned int>)
        err = nvs_set_u32(m_handle, key, value);
    else if constexpr(std::is_same_v<T, int32_t> || std::is_same_v<T, int>)
        err = nvs_set_i32(m_handle, key, value);
    else if constexpr(std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long>)
        err = nvs_set_u64(m_handle, key, value);
    else if constexpr(std::is_same_v<T, int64_t> || std::is_same_v<T, long long>)
        err = nvs_set_i64(m_handle, key, value);
    else {
        static_assert(always_false<T>, "Unsupported type");
    }

    if (err == ESP_OK)
        return eResult::SUCCESS;

    return eResult::FLASH_FAILURE;
}

} // namespace Baozi

#endif // __NVS_H__
