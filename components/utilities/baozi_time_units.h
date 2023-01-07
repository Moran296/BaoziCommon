#ifndef BAOZI_TIME_UNITS_H__
#define BAOZI_TIME_UNITS_H__

#include "freertos/FreeRTOS.h" //for TickType_t
#include <compare>
#include <concepts>

namespace Baozi
{

#define OVERLOAD_ALL_COMPARISON_OPERATORS(CLASS, MEMBER)    \
    constexpr auto operator<=>(const CLASS &rhs) const      \
    {                                                       \
        return MEMBER <=> rhs.MEMBER;                       \
    }                                                       \
    constexpr const CLASS operator-(const CLASS &rhs) const \
    {                                                       \
        return CLASS(MEMBER - rhs.MEMBER);                  \
    }                                                       \
    constexpr const CLASS operator+(const CLASS &rhs) const \
    {                                                       \
        return CLASS(MEMBER + rhs.MEMBER);                  \
    }                                                       \
    constexpr CLASS &operator+=(const CLASS &rhs)           \
    {                                                       \
        MEMBER += rhs.MEMBER;                               \
        return *this;                                       \
    }                                                       \
    constexpr CLASS &operator-=(const CLASS &rhs)           \
    {                                                       \
        MEMBER -= rhs.MEMBER;                               \
        return *this;                                       \
    }

    class Seconds;
    class MilliSeconds;
    class MicroSeconds;

    class Minutes
    {
    public:
        constexpr Minutes(uint32_t minutes) : m_minutes(minutes) {}
        constexpr Minutes() : m_minutes(0) {}
        constexpr operator Seconds();
        constexpr operator MilliSeconds();
        constexpr operator MicroSeconds();
        constexpr uint32_t value() const { return m_minutes; }

        OVERLOAD_ALL_COMPARISON_OPERATORS(Minutes, m_minutes)
        constexpr Minutes &operator=(Minutes const &) = default;
        constexpr Minutes(Minutes const &) = default;

    private:
        uint32_t m_minutes;
    };

    class Seconds
    {
    public:
        constexpr Seconds(uint32_t seconds) : m_seconds(seconds) {}
        constexpr Seconds() : m_seconds(0) {}
        constexpr operator MilliSeconds();
        constexpr operator Minutes();
        constexpr operator MicroSeconds();
        constexpr uint32_t value() const { return m_seconds; }
        constexpr TickType_t toTicks() const { return pdMS_TO_TICKS(m_seconds * 1000); }

        OVERLOAD_ALL_COMPARISON_OPERATORS(Seconds, m_seconds)
        constexpr Seconds &operator=(Seconds const &) = default;
        constexpr Seconds(Seconds const &) = default;

    private:
        uint32_t m_seconds;
    };

    class MilliSeconds
    {
    public:
        constexpr MilliSeconds(uint32_t ms) : m_ms(ms) {}
        constexpr MilliSeconds() : m_ms(0) {}
        constexpr operator Minutes();
        constexpr operator Seconds();
        constexpr operator MicroSeconds();
        constexpr TickType_t toTicks() const { return pdMS_TO_TICKS(m_ms); }

        constexpr uint32_t value() const { return m_ms; }

        OVERLOAD_ALL_COMPARISON_OPERATORS(MilliSeconds, m_ms)
        MilliSeconds &operator=(MilliSeconds const &) = default;
        MilliSeconds(MilliSeconds const &) = default;

    private:
        uint32_t m_ms;
    };

    class MicroSeconds
    {
    public:
        constexpr MicroSeconds(uint64_t us) : m_us(us) {}
        constexpr MicroSeconds() : m_us(0) {}
        constexpr operator Minutes();
        constexpr operator Seconds();
        constexpr operator MilliSeconds();
        constexpr uint64_t value() const { return m_us; }

        OVERLOAD_ALL_COMPARISON_OPERATORS(MicroSeconds, m_us)
        MicroSeconds &operator=(MicroSeconds const &) = default;
        MicroSeconds(MicroSeconds const &) = default;

    private:
        uint64_t m_us;
    };

    // user defined literals
    constexpr Minutes operator"" _min(unsigned long long int minutes)
    {
        return Minutes(minutes);
    }
    constexpr Seconds operator"" _sec(unsigned long long int seconds)
    {
        return Seconds(seconds);
    }
    constexpr MilliSeconds operator"" _ms(unsigned long long int ms)
    {
        return MilliSeconds(ms);
    }
    constexpr MicroSeconds operator"" _us(unsigned long long int us)
    {
        return MicroSeconds(us);
    }

    constexpr Minutes::operator Seconds() { return Seconds(m_minutes * 60); }
    constexpr Minutes::operator MilliSeconds() { return MilliSeconds(m_minutes * 60 * 1000); }
    constexpr Minutes::operator MicroSeconds() { return MicroSeconds(m_minutes * 60 * 1000 * 1000); }
    constexpr Seconds::operator Minutes() { return Minutes(m_seconds / 60); }
    constexpr Seconds::operator MilliSeconds() { return MilliSeconds(m_seconds * 1000); }
    constexpr Seconds::operator MicroSeconds() { return MicroSeconds(m_seconds * 1000 * 1000); }
    constexpr MilliSeconds::operator Minutes() { return Minutes(m_ms / 1000 / 60); }
    constexpr MilliSeconds::operator Seconds() { return Seconds(m_ms / 1000); }
    constexpr MilliSeconds::operator MicroSeconds() { return MicroSeconds(m_ms * 1000); }
    constexpr MicroSeconds::operator Seconds() { return Seconds(m_us / 1000 / 1000); }
    constexpr MicroSeconds::operator MilliSeconds() { return MilliSeconds(m_us / 1000); }
    constexpr MicroSeconds::operator Minutes() { return Minutes(m_us / 1000 / 1000 / 60); }

    template <typename T>
    concept TimeUnit = requires(T t) {
                           {
                               t.toTicks()
                               }
                               -> std::same_as<TickType_t>;
                       };

    void BaoDelay(const TimeUnit auto &time)
    {
        vTaskDelay(time.toTicks());
    }

} // Baozi

#endif