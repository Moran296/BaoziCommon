#ifndef UTIL_BAOZI_JSON_TRAITS_H__
#define UTIL_BAOZI_JSON_TRAITS_H__

#include <experimental/type_traits>
#include <type_traits>
#include "baozi_traits.h"
#include <optional>
#include <cJSON.h>

namespace Baozi
{

    class BaoJson; // forward declaration

    // ================== HAS VALUE ==================
    /*
        helper trait to check if a type has a value() method
        examples are reef_time_units (Seconds, Minutes, Hours, etc.) any other user defined units
    */

    template <class T>
    using has_value = decltype(std::declval<T &>().value());

    template <typename T>
    using HasValue = std::experimental::is_detected_convertible<double, has_value, T>;

    template <typename T>
    inline constexpr bool hasValue_v = HasValue<T>::value;

    // =================== HAS C STR =================
    /*
        helper trait to check if a type has a c_str() method
        examples are std::string, etl::string, etl enums
    */

    template <class T>
    using has_c_str = decltype(std::declval<T &>().c_str());

    template <typename T>
    using HasCStr = std::experimental::is_detected_exact<const char *, has_c_str, T>;

    template <typename T>
    inline constexpr bool HasCStr_v = HasCStr<T>::value;

    // =================== HAS TO JSON =================

    /*
        helper trait to check if a type has a ToJson() method
        examples are blob configs that must transform themselves into a json object
    */
    template <class T>
    using has_to_json = decltype(std::declval<T &>().ToJson());

    template <typename T>
    using HasToJson = std::experimental::is_detected_exact<BaoJson, has_to_json, T>;

    template <typename T>
    inline constexpr bool hasToJson_v = HasToJson<T>::value;

    // =================== IS JSON DESERIALIZABLE =================

    /*
        trait to define if a type is json deserializable.
        This is used in BaoJson::GetVal<T>() to check if the type of T is supported
    */
    template <typename T>
    struct is_json_deserializable : public std::__or_<std::is_arithmetic<T>,
                                                      std::is_same<T, const char *>,
                                                      std::is_same<T, char *>,
                                                      std::is_same<T, BaoJson>,
                                                      HasValue<T>,
                                                      HasCStr<T>,
                                                      std::is_same<T, bool>>::type
    {
    };

    // =================== IS OPTIONAL WITH JSON DESERIALIZABLE TYPE =================

    /*
        trait to define if a type is an optional with a json deserializable type.
    */

    template <typename T, typename = void>
    struct is_json_optional : public std::false_type
    {
    };

    template <typename T>
    struct is_json_optional<std::optional<T>> : public is_json_deserializable<T>::type
    {
    };

    // =================== IS JSON SERIALIZABLE =================

    /*
        trait to define if a type is json serializable.
        This is used in BaoJson::AddVal<T>() to check if the type of T can be added to a json object
    */
    template <typename T>
    struct is_json_serializable : public std::__or_<is_json_deserializable<T>,
                                                    is_json_optional<T>,
                                                    HasToJson<T>,
                                                    std::is_same<T, cJSON *>>::type
    {
    };

    template <typename T>
    inline constexpr bool is_json_serializable_v = is_json_serializable<T>::value;

    template <typename T>
    inline constexpr bool is_json_deserializable_v = is_json_deserializable<T>::value;

    // =================== IS KEY VALUE VALID =================

    /*
        KV struct is used to define key value pairs for BaoJson::AddVals() / BaoJson ctor with key-values
            example:
                  BaoJson json {
                     KV{"int", 5},
                     KV{"const char*", "hello"},
                     KV{"other json", BaoJson{}},
                     KV{"unit", Minute(1)},
                     KV{"optional with null", std::optional<int>(std::nullopt)},
                     KV{"boolean", true}};
    */
    template <typename T>
    struct KV
    {
        KV(const char *key, T value) : m_key(key), m_val(value) {}

        const char *m_key;
        T m_val;
    };

    /*
        helper trait to check if a type is a KV<T> struct
    */
    template <typename T, typename Enable = void>
    struct is_valid_key_value : public std::false_type
    {
    };

    template <typename T>
    struct is_valid_key_value<KV<T>> : public std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_valid_key_value_v = is_valid_key_value<T>::value;

} // namespace Baozi

#endif