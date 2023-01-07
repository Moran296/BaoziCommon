#ifndef UTIL_BAOZI_JSON_H__
#define UTIL_BAOZI_JSON_H__

#include <memory>
#include "cJSON.h"
#include "baozi_json_traits.h"
#include "freertos/FreeRTOS.h"

namespace Baozi {

class BaoJson
{
    public:
    using json_ptr_t = std::unique_ptr<cJSON, void (*)(void *)>;

    BaoJson();

    template <typename T, typename = std::enable_if_t<!is_valid_key_value<T>::value, void>>
    explicit BaoJson(T val);

    template <typename... Ts>
    BaoJson(Ts... vals);

    static BaoJson CreateArray();

    BaoJson Duplicate() const;

    std::optional<BaoJson> CopyItem(const char *key) const;

    static std::optional<BaoJson> Parse(const char *jsonAsStr);

    /*
        As the inner json is a unique pointer, copy constructor and assignment operator are deleted
        use ::Duplicate() instead if you need a copy
    */
    BaoJson(const BaoJson &rhs) = delete;
    BaoJson &operator=(const BaoJson &rhs) = delete;

    /*
        Move constructor and assignment operator are defaulted
        use std::move() if you need to move a BaoJson to a function (e.g. when a function parameter receives a BaoJson&&)
        You can still return a BaoJson from a function due to RVO (this allows creating the returned BaoJson in the caller function)
    */
    BaoJson(BaoJson &&rhs) = default;
    BaoJson &operator=(BaoJson &&rhs) = default;
    ~BaoJson() = default;

    /**
     * @brief return the inner cJSON
     * @brief NOTICE - Try to avoid these functions and use BaoJson API instead
     * @brief NOTICE - Use this functions if you need to use cJSON API that is not implemented in BaoJson
     *
     * @return cJSON*
     */
    cJSON *data();
    const cJSON *data() const;

    /**
     * @brief take the inner cJSON, leaving theBaoJson object empty and invalid!!!
     * @brief NOTICE - Try to avoid this if possible
     * @brief NOTICE - this returns a unique_pointer. It will release itself when it goes out of scope
     * @brief NOTICE - You can call json.take().release() to get the cJSON and free it yourself (with cJSON_Delete)
     *
     * @return json_ptr_t (std::unique_ptr<cJSON>)
     */
    [[nodiscard]] json_ptr_t take();

    /**
     * @brief return the inner cJSON as a string
     * @brief NOTICE - This allocates memory for the string. But does not modify the inner json
     * @brief NOTICE - this returns a unique_pointer. It will release itself when it goes out of scope
     *
     * @return unique_ptr<char>
     */
    std::unique_ptr<char, void (*)(void *)> PrintRaw() const;

    /**
     * @brief return true if the inner cJSON is not null
     */
    operator bool() const;

    /**
     * @brief return true if the inner cJSON in empty or null
     */
    bool IsEmpty() const;

    /**
     * @brief reset the unique pointer to null
     * @brief NOTICE - Try to avoid this if possible
     *
     */
    void reset();

    /**
     * @brief checks whether the json contains a property with the given key
     *
     * @param key - the key to check
     * @returns true if the json contains the key, false otherwise
     */
    bool HasItem(const char *key) const;

    /**
     * @brief checks whether the json contains either of the given keys
     *
     * @param keys - the keys to check
     * @returns true if the json contains either of the keys, false otherwise
     */
    template <typename... Ts>
    bool HasEitherItems(Ts... keys) const;

    /**
     * @brief checks whether the json contains all of the given keys
     *
     * @param keys - the keys to check
     * @returns true if the json contains all of the keys, false otherwise
     */
    template <typename... Ts>
    bool HasAllItems(Ts... keys) const;

    /**
     * @brief Adds a value of any json serializable type to the json array
     * @brief NOTICE - see serializable types in the reef_json_trais.h file
     *
     * @param key - key to add
     * @param val - val to add of any json serializable type
     *
     * @example
     *         BaoJson json;
     *         json.AddVal("int", 5);
     *         json.AddVal("const char*", "hello");
     *         json.AddVal("boolean", true);
     */
    template <typename T>
    void AddVal(const char *key, T val);

    /**
     * @brief Adds values to the json object
     * @brief NOTICE - Every property must be a KV struct of a const char* key and a json serializable value
     * @brief NOTICE - see serializable types in the reef_json_trais.h file
     *
     * @param vals - vals to add of any json serializable type
     *
     * @example
     *         BaoJson json;
     *         json.AddVals (
     *            KV{"int", 5},
     *            KV{"const char*", "hello"},
     *            KV{"other json", BaoJson{}},
     *            KV{"unit", Minute(1)},
     *            KV{"string", std::string("world")},
     *            KV{"optional with null", std::optional<int>(std::nullopt)},
     *            KV{"boolean", true});
     */
    template <typename... Ts>
    void AddVals(Ts... vals);

    /**
     * @brief Get a value from the json object by key
     *
     * @tparam T the type of the value to get
     * @param key the key of the value to get
     * @return std::optional<T> the value if it exists, std::nullopt otherwise
     *
     * @example
     *         //create a json with the following structure to test:
     *         BaoJson json {
     *            KV{"int", 5},
     *            KV{"const char*", "hello"},
     *            KV{"other json", BaoJson{}},
     *            KV{"unit", Minute(1)},
     *            KV{"optional with null", std::optional<int>(std::nullopt)},
     *            KV{"boolean", true}};
     *
     *           // type validation
     *           std::optional<int> = json.GetVal<int>("int"); // has value
     *           std::optional<const char*> = json.GetVal<const char*>("int"); // does not have value (wrong type)
     *           std::optional<int> = json.GetVal<int>("not_exists"); // does not have value (key does not exist)
     *           std::optional<Minute> = json.GetVal<Minute>("unit"); // has value, numbers would also work
     *           std::optional<BaoJson> = json.GetVal<Minute>("other json"); // has value
     */
    template <typename T>
    std::optional<T> GetVal(const char *key) const;

    /**
     * @brief Get a value from json if it is from this type
     * @brief NOTICE - this function works like GetVal, but without the need to specify the key
     *
     * @tparam T the type of the value to get
     * @return std::optional<T> the json is of T type, std::nullopt otherwise
     */
    template <typename T>
    std::optional<T> GetVal() const;

    /**
     * @brief Add an std::optional - if it has a value, add it, otherwise add null
     * @brief NOTICE - this function is used when you supply AddVal/s with optionals
     *
     * @param key the key to add
     * @param optionalVal the optional value to add
     */
    template <typename T>
    void AddValueOrNull(const char *key, std::optional<T> optionalVal);

    /**
     * @brief Add an std::optional - if it has a value, add it, otherwise add the supplied default value
     *
     * @param key the key to add
     * @param optionalVal the optional value to add
     * @param defaultVal the default value to add if the optional does not have a value
     */
    template <typename T>
    void AddValueOrDefault(const char *key, std::optional<T> optionalVal, T defaultVal);

    // ================== ARRAY FUNCTIONS ==================
    /**
     * @brief Adds a BaoJson to the json array
     * @brief NOTICE - This function is only valid for json arrays, will assert otherwise
     *
     * @tparam T - any json serializable type
     * @param val - the item to add
     */
    template <typename T>
    void AddValToArray(T val);

    // special case for adding BaoJson to array
    void AddValToArray(BaoJson &&val);

    /**
     * @brief Adds multiple values of any json serializable type to the json array
     *
     * @param vals collection of any json serializable type to add
     * @example
     *        BaoJson json = BaoJson::CreateArray();
     *        json.AddValsToArray(1, "2", 3.0, false, BaoJson{5});
     */
    template <typename... Ts>
    void AddValsToArray(Ts &&...vals);

    /**
     * @brief Adds multiple values of any json serializable type to the json array from input iterators
     *
     * @param begin - begin iterator
     * @param end - end iterator
     * @example
     *        BaoJson json = BaoJson::CreateArray();
     *        std::array<int, 5> arr{{1, 2, 3, 4, 5}};
     *         json.AddValsToArray(arr.begin(), arr.end());
     */
    template <class InputIt, typename = std::enable_if_t<std::is_base_of_v<std::input_iterator_tag,
                                                                           typename std::iterator_traits<InputIt>::iterator_category>,
                                                         void>>
    void AddValsToArray(InputIt begin, InputIt end);

    /**
     * @brief Performs a function for each item in array of type T
     *
     * @tparam T the type of the value to get
     * @tparam F a functor or a function that takes a T and returns void
     * @param func the function to perform
     *
     * @example
     *      BaoJson array = BaoJson::CreateArray();
     *      array.AddValsToArray(1, 2, 3, "hooray", BaoJson{});
     *      array.ArrayForEach<int>([](int i) { std::cout << i << std::endl; }); //prints 1, 2, 3
     *      array.ArrayForEach<const char *>([](const char *str) { std::cout << str << std::endl; }); //prints hooray
     *      array.ArrayForEach<BaoJson>([](BaoJson json) { std::cout << json << std::endl; }); //prints { }
     */
    template <typename T, typename F>
    void ArrayForEach(F &&func) const;

    /**
     * @brief Pop a value from the front/back of the array
     *
     * @return std::optional<T> the json if array is not empty, std::nullopt otherwise
     * @example
     *     BaoJson array = BaoJson::CreateArray();
     *     array.AddValsToArray(1, 2, 3, "hooray", BaoJson{});
     *     while(array.ArraySize() > 0)
     *        printf("popped = %s", array.ArrayPopFront().value().PrintRaw().get()); // prints 1, 2, 3, hooray, { }
     */
    std::optional<BaoJson> ArrayPopFront();
    std::optional<BaoJson> ArrayPopBack();

    /**
     * @brief Get size of array if an array
     *
     * @return size of array
     */
    int ArraySize() const;

    bool IsArray() const;

    private:

    json_ptr_t m_json;

    static void s_jsonFree(void *json);
};

}   // namespace Baozi

#include "baozi_json_inl.hpp"

#endif   // UTIL_CJSON_H__
