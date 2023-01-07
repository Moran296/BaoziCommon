#ifndef UTIL_BAOZI_JSON_INL_H__
#define UTIL_BAOZI_JSON_INL_H__

#include "baozi_json_traits.h"

namespace Baozi {

// helper specialization for adding json objects when BaoJson is an rvalue
template <>
struct KV<BaoJson>
{
    KV(const char *key, BaoJson &&value) : m_key(key), m_val(value.take().release()) {}

    const char *m_key;
    cJSON *m_val;
};

template <typename... Ts>
BaoJson::BaoJson(Ts... vals) : BaoJson() {
    AddVals(vals...);
}

template <typename T, typename>
BaoJson::BaoJson(T val) : m_json(nullptr, s_jsonFree) {
    static_assert(is_json_serializable_v<T>, "type is not json serializable");

    if constexpr (std::is_same_v<T, bool>) {
        m_json.reset(cJSON_CreateBool(val));
    } else if constexpr (std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>) {
        m_json.reset(cJSON_CreateNumber(val));
    } else if constexpr (std::is_same_v<T, const char *>) {
        m_json.reset(cJSON_CreateString(val));
    } else if constexpr (std::is_same_v<T, char *>) {
        m_json.reset(cJSON_CreateString(val));
    } else if constexpr (std::is_same_v<T, cJSON *>) {
        m_json.reset(val);
    } else if constexpr (HasCStr_v<T>) {
        m_json.reset(cJSON_CreateString(val.c_str()));
    } else if constexpr (is_json_optional<T>::value) {
        if (val.has_value())
            m_json.reset(BaoJson{ val.value() }.take().release());
        else
            m_json.reset(cJSON_CreateNull());
    } else if constexpr (hasValue_v<T>) {
        m_json.reset(BaoJson{ val.value() }.take().release());
    } else if constexpr (hasToJson_v<T>) {
        m_json.reset(val.ToJson().take().release());
    } else {
        static_assert(always_false<T>, "type cant be deduced");
    }

    configASSERT(m_json != nullptr);
}

template <typename... Ts>
bool BaoJson::HasEitherItems(Ts... keys) const {
    return (HasItem(keys) || ...);
}

// Has all the keys in the list
template <typename... Ts>
bool BaoJson::HasAllItems(Ts... keys) const {
    return (HasItem(keys) && ...);
}

template <typename T>
void BaoJson::AddVal(const char *key, T val) {
    static_assert(is_json_serializable_v<T>, "type is not json serializable");

    if constexpr (std::is_same_v<T, bool>) {
        configASSERT(cJSON_AddBoolToObject(m_json.get(), key, val) != nullptr);
    } else if constexpr (std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>) {
        configASSERT(cJSON_AddNumberToObject(m_json.get(), key, val) != nullptr);
    } else if constexpr (std::is_same_v<T, const char *>) {
        configASSERT(cJSON_AddStringToObject(m_json.get(), key, val) != nullptr);
    } else if constexpr (std::is_same_v<T, char *>) {
        configASSERT(cJSON_AddStringToObject(m_json.get(), key, val) != nullptr);
    } else if constexpr (std::is_same_v<T, cJSON *>) {
        cJSON_AddItemToObject(m_json.get(), key, val);
    } else if constexpr (HasCStr_v<T>) {
        configASSERT(cJSON_AddStringToObject(m_json.get(), key, val.c_str()) != nullptr);
    } else if constexpr (is_json_optional<T>::value) {
        AddValueOrNull(key, val);
    } else if constexpr (hasValue_v<T>) {
        configASSERT(cJSON_AddNumberToObject(m_json.get(), key, val.value()) != nullptr);
    } else if constexpr (hasToJson_v<T>) {
        AddVal(key, val.ToJson().take().release());
    } else if constexpr (std::is_same_v<T, BaoJson>) {
        AddVal(key, val.take().release());
    } else {
        static_assert(always_false<T>, "Could not deduce type");
    }
}

template <typename... Ts>
void BaoJson::AddVals(Ts... vals) {
    static_assert((is_valid_key_value_v<Ts> && ...), "invalid key value");
    (AddVal(vals.m_key, vals.m_val), ...);
}

template <typename T>
std::optional<T> BaoJson::GetVal(const char *key) const {
    static_assert(is_json_deserializable_v<T>, "Type is not JSON deserializable");

    cJSON *item = cJSON_GetObjectItem(m_json.get(), key);
    if (item == nullptr) {
        return std::nullopt;
    }

    if constexpr (std::is_same_v<T, bool>)
        return cJSON_IsBool(item) ? std::optional{ (bool)cJSON_IsTrue(item) } : std::nullopt;
    else if constexpr (std::is_arithmetic_v<T>)
        return cJSON_IsNumber(item) ? std::optional{ cJSON_GetNumberValue(item) } : std::nullopt;
    else if constexpr (std::is_same_v<T, const char *>)
        return cJSON_IsString(item) ? std::optional{ cJSON_GetStringValue(item) } : std::nullopt;
    else if constexpr (std::is_same_v<T, BaoJson>)
        return (cJSON_IsObject(item) || cJSON_IsArray(item) || cJSON_IsNull(item)) ? std::optional{ BaoJson(cJSON_DetachItemViaPointer(m_json.get(), item)) } : std::nullopt;
    else if constexpr (hasValue_v<T>)
        return cJSON_IsNumber(item) ? std::optional{ T(cJSON_GetNumberValue(item)) } : std::nullopt;
    else if constexpr (HasCStr_v<T>)
        return cJSON_IsString(item) ? std::optional{ T{ cJSON_GetStringValue(item) } } : std::nullopt;
    else
        return std::nullopt;
}

template <typename T>
std::optional<T> BaoJson::GetVal() const {
    static_assert(is_json_deserializable_v<T>, "Type is not JSON deserializable");

    if (m_json.get() == nullptr) {
        return std::nullopt;
    }

    if constexpr (std::is_same_v<T, bool>)
        return cJSON_IsBool(m_json.get()) ? std::optional{ (bool)cJSON_IsTrue(m_json.get()) } : std::nullopt;
    else if constexpr (std::is_arithmetic_v<T>)
        return cJSON_IsNumber(m_json.get()) ? std::optional{ cJSON_GetNumberValue(m_json.get()) } : std::nullopt;
    else if constexpr (std::is_same_v<T, const char *>)
        return cJSON_IsString(m_json.get()) ? std::optional{ cJSON_GetStringValue(m_json.get()) } : std::nullopt;
    else if constexpr (hasValue_v<T>)
        return cJSON_IsNumber(m_json.get()) ? std::optional{ T(cJSON_GetNumberValue(m_json.get())) } : std::nullopt;
    else if constexpr (HasCStr_v<T>)
        return cJSON_IsString(m_json.get()) ? std::optional{ T{ cJSON_GetStringValue(m_json.get()) } } : std::nullopt;
    else
        return std::nullopt;
}

template <typename T>
void BaoJson::AddValueOrNull(const char *key, std::optional<T> optionalVal) {
    static_assert(is_json_deserializable_v<T>, "Type is not JSON deserializable");

    if (!optionalVal.has_value()) {
        configASSERT(cJSON_AddNullToObject(m_json.get(), key) != nullptr);
    } else {
        AddVal(key, optionalVal.value());
    }
}

template <typename T>
void BaoJson::AddValueOrDefault(const char *key, std::optional<T> optionalVal, T defaultVal) {
    static_assert(is_json_deserializable_v<T>, "Type is not JSON deserializable");

    if (!optionalVal.has_value()) {
        AddVal(key, defaultVal);
    } else {
        AddVal(key, optionalVal.value());
    }
}

template <typename T>
void BaoJson::AddValToArray(T val) {
    configASSERT(cJSON_IsArray(m_json.get()));

    auto item = BaoJson(val);
    configASSERT(cJSON_AddItemToArray(m_json.get(), item.take().release()) == true);
}

template <typename... Ts>
void BaoJson::AddValsToArray(Ts &&...vals) {
    (AddValToArray(std::forward<Ts>(vals)), ...);
}

// //AddVals from iterators
template <class InputIt, typename>
void BaoJson::AddValsToArray(InputIt begin, InputIt end) {
    for (auto it = begin; it != end; ++it) {
        AddValToArray(*it);
    }
}

template <typename T, typename F>
void BaoJson::ArrayForEach(F &&func) const {
    configASSERT(cJSON_IsArray(data()));
    static_assert(is_json_deserializable_v<T>, "Type is not JSON deserializable");

    if (m_json.get() == nullptr) {
        return;
    }

    cJSON *item = m_json.get()->child;
    while (item != nullptr) {
        if constexpr (std::is_same_v<T, bool>) {
            if (cJSON_IsBool(item))
                func(cJSON_IsTrue(item));
        } else if constexpr (std::is_same_v<BaoJson, T>) {
            if (cJSON_IsObject(item)) {
                auto temp = BaoJson{ item };
                func(temp);
                temp.m_json.release();
            }
        } else if constexpr (std::is_same_v<cJSON *, T>) {
            if (cJSON_IsObject(item))
                func(item);
        } else if constexpr (std::is_arithmetic_v<T>) {
            if (cJSON_IsNumber(item))
                func(cJSON_GetNumberValue(item));
        } else if constexpr (std::is_same_v<T, const char *>) {
            if (cJSON_IsString(item))
                func(cJSON_GetStringValue(item));
        } else if constexpr (hasValue_v<T>) {
            if (cJSON_IsNumber(item))
                func(T{ cJSON_GetNumberValue(item) });
        } else if constexpr (HasCStr_v<T>) {
            if (cJSON_IsString(item))
                func(cJSON_GetStringValue(T{ item }));
        }

        item = item->next;
    }
}

}   // namespace Baozi

#endif   // BAOZI_JSON_INL_H