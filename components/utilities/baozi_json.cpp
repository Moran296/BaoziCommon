#include "baozi_json.h"

namespace Baozi {

BaoJson::BaoJson() : m_json(cJSON_CreateObject(), s_jsonFree) {
    configASSERT(m_json != nullptr);
}

BaoJson BaoJson::CreateArray() {
    return BaoJson(cJSON_CreateArray());
}

BaoJson BaoJson::Duplicate() const {
    return BaoJson(cJSON_Duplicate(m_json.get(), true));
}

std::optional<BaoJson> BaoJson::CopyItem(const char *key) const {
    if (cJSON *item = cJSON_GetObjectItem(m_json.get(), key); item != nullptr)
        return BaoJson(cJSON_Duplicate(item, true));

    return std::nullopt;
}

std::optional<BaoJson> BaoJson::Parse(const char *jsonAsStr) {
    if (cJSON *json = cJSON_Parse(jsonAsStr); json != nullptr) {
        auto rj = BaoJson{ json };
        return std::optional<BaoJson>(std::move(rj));
    }

    return std::nullopt;
}

cJSON *BaoJson::data() {
    return m_json.get();
}

const cJSON *BaoJson::data() const {
    return m_json.get();
}

BaoJson::json_ptr_t BaoJson::take() {
    return std::move(m_json);
}

std::unique_ptr<char, void (*)(void *)> BaoJson::PrintRaw() const {
    return { cJSON_PrintUnformatted(m_json.get()), cJSON_free };
}

BaoJson::operator bool() const {
    return m_json != nullptr;
}

bool BaoJson::IsEmpty() const {
    return m_json != nullptr && m_json->child == nullptr;
}

void BaoJson::reset() {
    m_json.reset();
}

bool BaoJson::HasItem(const char *key) const {
    return cJSON_HasObjectItem(m_json.get(), key);
}

void BaoJson::AddValToArray(BaoJson &&val) {
    configASSERT(cJSON_IsArray(m_json.get()));

    configASSERT(cJSON_AddItemToArray(m_json.get(), val.take().release()) == true);
}

std::optional<BaoJson> BaoJson::ArrayPopFront() {
    configASSERT(cJSON_IsArray(data()));
    cJSON *first = cJSON_DetachItemFromArray(data(), 0);
    return first ? std::make_optional(BaoJson{ first }) : std::nullopt;
}

std::optional<BaoJson> BaoJson::ArrayPopBack() {
    configASSERT(cJSON_IsArray(data()));
    cJSON *last = cJSON_DetachItemFromArray(data(), cJSON_GetArraySize(data()) - 1);
    return last ? std::make_optional(BaoJson{ last }) : std::nullopt;
}

int BaoJson::ArraySize() const {
    configASSERT(cJSON_IsArray(data()));
    return cJSON_GetArraySize(data());
}

bool BaoJson::IsArray() const {
    return cJSON_IsArray(data());
}


// =====================================================================

void BaoJson::s_jsonFree(void *json) {
    cJSON_Delete(static_cast<cJSON *>(json));
}


}   // namespace Baozi
