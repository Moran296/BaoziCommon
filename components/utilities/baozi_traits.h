#ifndef BAOZI_TRAITS_H__
#define BAOZI_TRAITS_H__

namespace Baozi
{

    // always false - used for static_asserts
    template <typename...>
    inline constexpr bool always_false = false;

} // Baozi

#endif