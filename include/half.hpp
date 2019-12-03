#ifndef HALF_INCLUDED
#define HALF_INCLUDED

#if __cplusplus < 201703L
    #error "Requires C++17 or later."
#endif

#if __has_include(<emmintrin.h>)
    #include <emmintrin.h>
#endif

#include <cstdint>
#include <cstddef>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>
#if __cplusplus > 201703L
    #include <bit>
#endif

namespace half_detail {
    #if __cplusplus > 201703L
        using std::bit_cast;
    #else
        template <typename To, typename From>
        auto bit_cast(From from)
        -> To
        {
            static_assert(std::is_trivial_v<From>);
            static_assert(std::is_trivial_v<To>);
            static_assert(sizeof(To) == sizeof(From));

            To to;
            std::memcpy(&to, &from, sizeof(to));
            return to;
        }
    #endif


    //template <typename T>
    //struct float_traits;

    //template <>
    //struct float_traits<float> {
    //    static constexpr

    //};

    template <typename T>
    auto bits_from_floating_point(T value)
    -> std::uint16_t
    {
        return {};
    }

    template <typename T>
    auto bits_from_integral(T value)
    -> std::uint16_t
    {
        return {};
    }

    template <typename T>
    auto bits_from_unsigned_integral(T value)
    -> std::uint16_t
    {
        return {};
    }

    template <typename T>
    auto bits_to_floating_point(std::uint16_t bits)
    -> T
    {
        return {};
    }

    template <typename T>
    auto bits_to_integral(std::uint16_t bits)
    -> T
    {
        return {};
    }

    template <typename T>
    auto bits_to_unsigned_integral(std::uint16_t bits)
    -> T
    {
        return {};
    }

    //single_to_half() {
    //    #ifdef __F16C__
    //        if (!std::is_constant_evaluated()) {
    //            half::bits = _mm_cvtss_sh(value, _MM_FROUND_CUR_DIRECTION);
    //            return;
    //        }
    //    #endif

    //    constexpr
    //    auto width = sizeof(T) * CHAR_BIT;

    //    using uint_fast_t = uint_fastN_t<width>;
    //    using uint_t = uintN_t<width>;

    //    constexpr
    //    auto frac_width = std::numeric_limits<T>::digits - 1;

    //    constexpr
    //    uint_fast_t tiny_exp_frac = uint_fast_t{1} << (frac_width - half::frac_width);

    //    constexpr
    //    uint_fast_t huge_exp_frac = uint_fast_t(half::max_exp - 1) << frac_width
    //                              | half::max_frac << (frac_width - half::frac_width);

    //    constexpr
    //    uint_fast_t exp_frac_mask = (uint_fast_t{1} << (width - 1)) - 1;

    //    unit_fast_t bits = std::bit_cast<uint_t>(x);
    //    uint_fast_t exp_frac = bits & exp_frac_mask;

    //    uint_fast_t underflow = -uint_fast_t(exp_frac < tiny_exp_frac);
    //    uint_fast_t overflow = -uint_fast_t(exp_frac > huge_exp_frac);
    //}

    //half_to_single() {
    //    #ifdef __F16C__
    //        if (!std::is_constant_evaluated()) {
    //            return _mm_cvtsh_ss(bits);
    //        }
    //    #endif
    //}
}

struct half {
    template <typename T, std::enable_if_t<
        std::is_floating_point_v<T>
    >*...>
    half(T value)
    : bits(half_detail::bits_from_floating_point(value))
    {}

    template <typename T, std::enable_if_t<
        std::is_integral_v<T> && !std::is_unsigned_v<T>
    >*...>
    half(T value)
    : bits(half_detail::bits_from_integral(value))
    {}

    template <typename T, std::enable_if_t<
        std::is_integral_v<T> && std::is_unsigned_v<T>
    >*...>
    half(T value)
    : bits(half_detail::bits_from_unsigned_integral(value))
    {}

    half()
    = default;

    template <typename T, std::enable_if_t<
        std::is_floating_point_v<T>
    >*...>
    operator T()
    const
    { return half_detail::bits_to_floating_point<T>(bits); }

    template <typename T, std::enable_if_t<
        std::is_integral_v<T> && !std::is_unsigned_v<T>
    >*...>
    operator T()
    const
    { return half_detail::bits_to_integral<T>(bits); }

    template <typename T, std::enable_if_t<
        std::is_integral_v<T> && std::is_unsigned_v<T>
    >*...>
    operator T()
    const
    { return half_detail::bits_to_unsigned_integral<T>(bits); }

private:
    std::uint16_t bits;
};

static_assert(sizeof(half) * CHAR_BIT == 16);
static_assert(std::is_trivial_v<half>);

namespace std {
    template <>
    class numeric_limits<::half> {
    public:


    };
}

inline
auto operator+(half x)
-> half
{ return x; }

inline
auto operator-(half x)
-> half
{
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(x) ^ 0b10000000u
        )
    );
}

inline
auto fabs(half x)
-> half
{
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(x) & 0b01111111u
        )
    );
}

inline
auto abs(half x)
-> half
{ return fabs(x); }

inline
auto signbit(half x)
-> bool
{ return half_detail::bit_cast<std::uint16_t>(x) & 0b10000000u; }

inline
auto copysign(half x, half y)
-> half
{
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(y) & 0b10000000u |
            half_detail::bit_cast<std::uint16_t>(x) & 0b01111111u
        )
    );
}

#endif
