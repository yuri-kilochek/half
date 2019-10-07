#ifndef HALF_DETAIL_INCLUDE_GUARD
#define HALF_DETAIL_INCLUDE_GUARD

#include <cstdint>
#include <cstddef>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>

class half {
    using uint_t = std::uint16_t;
    using uint_fast_t = std::uint_fast16_t;

    uint_t bits;

    template <typename T>
    struct float_layout {
        static constexpr
        unsigned const width = sizeof(T) * CHAR_BIT;

        static constexpr
        unsigned const mantissa_width = std::numeric_limits<T>::digits;

        static constexpr
        auto const exponent_width = width - mantissa_width - 1u;

        static constexpr
        auto const exponent_offset = mantissa_width;

        static constexpr
        auto const sign_offset = width - 1u;


    };




    // TODO: C++2a: Replace with `std::bit_cast`.
    template <typename To typename From>
    static
    auto bit_cast(From from)
    -> To
    {
        To to;
        std::memcpy(&to, &from, sizeof(to));
        return to;
    }

public:
    template <typename T,
        std::enable_if_t<std::numeric_limits<T>::is_iec559>*...>
    half(T x) {
        using layout_t = float_layout<T>;

        using uint_fast_t = uint_fastN_t<layout_t::width>;
        using uint_t = uintN_t<layout_t::width>;

        unit_fast_t bits = bit_cast<uint_t>(x);

        auto sign = half::fast_uint_t(bits >> (layout::sign_offset - half::layout::sign_offset)) & half::sign_mask;
        auto exponent = bits >> mantissa_width & bits_t{0b11111111};
        auto mantissa = bits & bits_t{0b11111111};

        half::bits = bits >> 16 & 



    }

    //template <typename Float64, std::enable_if_t<is_ieee754_binary_v<Float64, 64>>*...>
    //half(Float64 float64) {
    //    std::uint_fast64_t bits64 = bit_cast<std::uint64_t>(float64);


    //}

    //template <typename Integer, std::enable_if_t<std::is_integral_v<Integer>>*...>
    //half(Integer integer) {

    //}

    half() = default;

    //template <typename Float32, std::enable_if_t<is_ieee754_binary_v<Float32, 32>>*...>
    //operator Float32()
    //const
    //{

    //}

    //template <typename Float64, std::enable_if_t<is_ieee754_binary_v<Float64, 64>>*...>
    //operator Float64()
    //const
    //{

    //}

    friend
    auto operator-(half x)
    -> half
    {
        half y;
        y.bits = x.bits ^ sign_mask;
        return y;
    }

    friend
    auto abs(half x)
    -> half
    {
        half y;
        y.bits = x.bits & ~sign_mask;
        return y;
    }
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
auto fabs(half x)
-> half
{ return abs(x); }

#endif
