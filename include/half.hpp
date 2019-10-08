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

    static constexpr
    std::size_t const exp_width = 5;

    static constexpr
    std::size_t const frac_width = 10;

    static constexpr
    uint_fast_t const max_exp = (uint_fast_t{1} << exp_width) - 1;

    static constexpr
    uint_fast_t const max_frac = (uint_fast_t{1} << frac_width) - 1;

    uint_t bits;


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
        constexpr
        auto width = sizeof(T) * CHAR_BIT;

        using uint_fast_t = uint_fastN_t<width>;
        using uint_t = uintN_t<width>;

        constexpr
        auto frac_width = std::numeric_limits<T>::digits - 1;

        constexpr
        uint_fast_t tiny_exp_frac = uint_fast_t{1} << (frac_width - half::frac_width);

        constexpr
        uint_fast_t huge_exp_frac = uint_fast_t(half::max_exp - 1) << frac_width
                                  | half::max_frac << (frac_width - half::frac_width);

        constexpr
        uint_fast_t exp_frac_mask = (uint_fast_t{1} << (width - 1)) - 1;

        unit_fast_t bits = bit_cast<uint_t>(x);
        uint_fast_t exp_frac = bits & exp_frac_mask;

        uint_fast_t underflow = -uint_fast_t(exp_frac < tiny_exp_frac);
        uint_fast_t overflow = -uint_fast_t(exp_frac > huge_exp_frac);




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
