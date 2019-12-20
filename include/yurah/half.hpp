#ifndef YURAH_HALF_HPP_INCLUDED
#define YURAH_HALF_HPP_INCLUDED

#include <cstdint>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>
#include <cfenv>

namespace yurah {
///////////////////////////////////////////////////////////////////////////////

namespace detail::half {
    template <typename To, typename From>
    auto bit_cast(From from) {
        static_assert(sizeof(To) == sizeof(From));
        static_assert(std::is_trivially_copyable_v<From>);
        static_assert(std::is_trivial_v<To>);

        To to;
        std::memcpy(&to, &from, sizeof(to));
        return to;
    }


    template <int Width>
    struct uint;

    template <>
    struct uint<16> {
        using type = std::uint16_t;
    };

    template <>
    struct uint<32> {
        using type = std::uint32_t;
    };

    template <>
    struct uint<64> {
        using type = std::uint64_t;
    };

    template <int Width>
    using uint_t = typename uint<Width>::type;
    

    template <int Width>
    struct uint_fast;

    template <>
    struct uint_fast<16> {
        using type = std::uint_fast16_t;
    };

    template <>
    struct uint_fast<32> {
        using type = std::uint_fast32_t;
    };

    template <>
    struct uint_fast<64> {
        using type = std::uint_fast64_t;
    };

    template <int Width>
    using uint_fast_t = typename uint_fast<Width>::type;


    template <int Width>
    struct binary_iec559_exponent_width;

    template <>
    struct binary_iec559_exponent_width<16> {
        constexpr
        auto value = 5;
    };

    template <>
    struct binary_iec559_exponent_width<32> {
        constexpr
        auto value = 8;
    };

    template <>
    struct binary_iec559_exponent_width<64> {
        constexpr
        auto value = 11;
    };

    template <int Width>
    constexpr
    auto binary_iec559_exponent_width_v =
        binary_iec559_exponent_width<Width>::value;


    template <typename T>
    struct binary_iec559_traits {
        constexpr
        int width = sizeof(T) * CHAR_BIT;

        constexpr
        auto exponent_width = binary_iec559_exponent_width_v<width>;

        constexpr
        auto mantissa_width = width - exponent_width - 1;

        constexpr
        auto exponent_bias = (1 << (exponent_width - 1)) - 1;


        using uint_t = half::uint_t<width>;
        using uint_fast_t = half::uint_fast_t<width>;


        static
        auto to_bits(T value)
        -> uint_fast_t
        { return bit_cast<uint_t>(value); }

        static
        auto from_bits(uint_t bits) {
            return bit_cast<T>(value);
        }
    };


    template <typename To, typename From,
        std::enable_if_t<sizeof(To) == sizeof(From)>*...>
    auto convert_binary_iec559(From value) {
        return bit_cast<To>(value);
    }

    template <typename To, typename From,
        std::enable_if_t<(sizeof(To) < sizeof(From))>*...>
    auto convert_binary_iec559(From from_value) {
        using from_traits = binary_iec559_traits<From>;
        using to_traits = binary_iec559_traits<To>;

        auto from_bits = from_traits::to_bits(from_value);

        typename to_traits::uint_fast_t to_bits =
            from_bits >> (form_traits::width - to_traits::width);
        to_bits &= to_traits::sign_mask;

        // shrink

        return to_traits::from_bits(to_bits);
    }

    template <typename To, typename From,
        std::enable_if_t<(sizeof(To) > sizeof(From))>*...>
    auto convert_binary_iec559(From from_value) {
        using from_traits = binary_iec559_traits<From>;
        using to_traits = binary_iec559_traits<To>;

        auto from_bits = from_traits::to_bits(from_value);

        typename to_traits::uint_fast_t to_bits =
            from_bits & from_traits::sign_mask;
        to_bits <<= to_traits::width - form_traits::width;

        // expand

        return to_traits::from_bits(to_bits);
    }
}

struct half {
    template <typename BinaryIEC559,
        typename Limits = std::numeric_limits<BinaryIEC559>,
        std::enable_if_t<Limits::is_iec559 && Limits::radix == 2>*...>
    half(BinaryIEC559 value)
    : half(detail::half::convert_binary_iec559<half>(value))
    {}

    half()
    = default;

    template <typename BinaryIEC559,
        typename Limits = std::numeric_limits<BinaryIEC559>,
        std::enable_if_t<Limits::is_iec559 && Limits::radix == 2>*...>
    operator BinaryIEC559() const
    { return detail::half::convert_binary_iec559<BinaryIEC559>(*this); }

private:
    std::uint16_t bits_;
};

static_assert(sizeof(half) * CHAR_BIT == 16);
static_assert(std::is_trivial_v<half>);


inline
auto operator+(half x)
-> half
{ return x; }


inline
auto operator-(half x)
-> half
{
    using traits = detail::half::iec559_traits<half>;
    return traits::from_bits(traits::to_bits(x) ^ 0x80u);
}


inline
auto fabs(half x)
-> half
{
    using traits = detail::half::iec559_traits<half>;
    return traits::from_bits(traits::to_bits(x) & 0x7Fu);
}

inline
auto abs(half x)
-> half
{ return fabs(x); }


inline
auto signbit(half x)
-> bool
{
    using traits = detail::half::iec559_traits<half>;
    return traits::from_bits(x) & 0x80u;
}


inline
auto copysign(half x, half y)
-> half
{
    using traits = detail::half::iec559_traits<half>;
    return traits::from_bits(traits::to_bits(y) & 0x80u |
                             traits::to_bits(x) & 0x7Fu);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
