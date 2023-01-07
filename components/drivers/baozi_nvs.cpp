#include "baozi_nvs.h"

namespace Baozi {

bool NVS::s_isInitialized = false;
char NVS::s_stringBuffer[256];

NVS::NVS(const char *nvs_namespace) : m_mutex()
{
    if (!s_isInitialized)
        init();

    esp_err_t err = nvs_open(nvs_namespace, NVS_READWRITE, &m_handle);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS open failed. esp err = %d", err);
        configASSERT(false);
    }

    BAO_LOG_INFO("NVS %s opened", nvs_namespace);
}

eResult NVS::init()
{
    esp_err_t err = ESP_OK;
    if (s_isInitialized)
    {
        BAO_LOG_WARNING("trying to double init NVS");
        return eResult::INVALID_STATE;
    }

    err = nvs_flash_init();
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("failed init of nvs err %d", err);
        configASSERT(false);
    }

    s_isInitialized = true;
    return eResult::SUCCESS;
}

eResult NVS::GetString(const char *key, char *value, size_t maxLen)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t requiredLen = 0;
    esp_err_t err = nvs_get_str(m_handle, key, nullptr, &requiredLen);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            return eResult::NOT_FOUND;
        }

        BAO_LOG_ERROR("NVS get string failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }

    if (requiredLen > maxLen)
    {
        BAO_LOG_ERROR("read string, buffer size = %d is smaller than value len = %d", maxLen, requiredLen);
        return eResult::INVALID_PARAMETER;
    }

    err = nvs_get_str(m_handle, key, value, &requiredLen);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS get string failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }

    return eResult::SUCCESS;
}

eResult NVS::SetString(const char *key, const char *value)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err = nvs_set_str(m_handle, key, value);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS set string failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }

    return eResult::SUCCESS;
}

eResult NVS::Erase()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err = nvs_erase_all(m_handle);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS erase failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }
    return eResult::SUCCESS;
}

eResult NVS::Erase(const char *key)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err = nvs_erase_key(m_handle, key);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS erase failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }
    return eResult::SUCCESS;
}

eResult NVS::SetBlob(const char *key, const void *value, size_t len)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err = nvs_set_blob(m_handle, key, value, len);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("NVS set blob failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }

    BAO_LOG_DEBUG("NVS set blob success");
    return eResult::SUCCESS;
}

eResult NVS::GetBlob(const char *key, void *value, size_t maxLen, size_t &actualLen)
{
    esp_err_t err = ESP_OK;
    std::lock_guard<std::mutex> lock(m_mutex);

    err = nvs_get_blob(m_handle, key, NULL, &actualLen);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            return eResult::NOT_FOUND;
        }

        BAO_LOG_ERROR("NVS get size of blob operation failed. esp err = %d", err);
        return eResult::FLASH_FAILURE;
    }

    if (actualLen > maxLen)
    {
        BAO_LOG_ERROR("size of requested blob is bigger than max requested");
        return eResult::INVALID_PARAMETER;
    }

    err = nvs_get_blob(m_handle, key, value, &actualLen);
    if (err != ESP_OK)
    {
        return eResult::FLASH_FAILURE;
        BAO_LOG_ERROR("NVS get blob operation failed. esp err = %d", err);
    }

    BAO_LOG_DEBUG("NVS get blob len = %d success", actualLen);
    return eResult::SUCCESS;
}

eResult NVS::Commit()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    esp_err_t err = nvs_commit(m_handle);
    if (err != ESP_OK)
    {
        BAO_LOG_ERROR("commit change to nvs failed, err %d", err);
        return eResult::FLASH_FAILURE;
    }

    BAO_LOG_INFO("NVS commit success");
    return eResult::SUCCESS;
}

} // namespace Baozi
