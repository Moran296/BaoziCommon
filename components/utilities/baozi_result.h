#ifndef BAOZI_RESULT_H__
#define BAOZI_RESULT_H__

#include <variant>
#include <concepts>
#include "baozi_traits.h"

namespace Baozi {

enum class eResult {
    SUCCESS,
    FAIL,
    FLASH_FAILURE,
    NOT_FOUND,
    INVALID_PARAMETER,
    INVALID_STATE,
    INVALID_OPERATION,
    TIMEOUT,
    OUT_OF_MEMORY,
    NOT_IMPLEMENTED,
    UNKNOWN,
};

template <class OK, class ERROR = eResult>
class BaoResult
{
    private:
        struct CtorKey{};
        BaoResult(CtorKey&&, OK ok) : m_result(ok) {}
        BaoResult(CtorKey&&, ERROR err) : m_result(err) {}

    public:
        template <class T>
        BaoResult(T&&) {
            static_assert(always_false<T>, "BaoResult can only be constructed with a single argument");
        }

        static BaoResult Ok(OK ok) { return BaoResult(CtorKey{}, ok); }
        static BaoResult Error(ERROR err) { return BaoResult(CtorKey{}, err); }

        ~BaoResult() = default;
        BaoResult(const BaoResult& other) = default;
        BaoResult& operator=(const BaoResult& other) = default;
        BaoResult(BaoResult&& other) = default;
        BaoResult& operator=(BaoResult&& other) = default;


        bool has_value() const { return std::holds_alternative<OK>(m_result); }
        bool has_error() const { return std::holds_alternative<ERROR>(m_result); }
        explicit operator bool () const { return has_value(); }

        OK& value() & { return std::get<OK>(m_result); }
        const OK& value() const& { return std::get<OK>(m_result); }
        OK&& value() && { return std::get<OK>(std::move(m_result)); }
        const OK&& value() const&& { return std::get<OK>(std::move(m_result)); }

        ERROR& error() & { return std::get<ERROR>(m_result); }
        const ERROR& error() const& { return std::get<ERROR>(m_result); }
        ERROR&& error() && { return std::get<ERROR>(std::move(m_result)); }
        const ERROR&& error() const&& { return std::get<ERROR>(std::move(m_result)); }

        // template <std::convertible_to<OK> U>
        // OK&& value_or(U&& default_value) && { return has_value() ? std::move(value()) : static_cast<OK>(std::forward<U>(default_value)); }

        template <std::convertible_to<OK> U>
        OK value_or(U&& default_value) { return has_value() ? value() : static_cast<OK>(std::forward<U>(default_value)); }

        // Error comparison operators
        bool operator==(const ERROR& other) const { return has_error() && error() == other; }
        bool operator!=(const ERROR& other) const { return !(*this == other); }
        // OK comparison operators
        bool operator==(const OK& other) const { return has_value() && value() == other; }
        bool operator!=(const OK& other) const { return !(*this == other); }

    private:
        std::variant <OK, ERROR> m_result;
};


} // Baozi

#endif
