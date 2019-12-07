#ifndef HALFFLOAT_DETAIL_INCLUDED_HPP
#define HALFFLOAT_DETAIL_INCLUDED_HPP

#include <cstdint>
#include <cstddef>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>

namespace halffloat {
///////////////////////////////////////////////////////////////////////////////

namespace detail {
    template <typename To, typename From>
    auto bit_cast(From from)
    -> To
    {
        static_assert(sizeof(To) == sizeof(From));
        static_assert(std::is_trivially_copyable_v<From>);
        static_assert(std::is_trivial_v<To>);

        To to;
        std::memcpy(&to, &from, sizeof(to));
        return to;
    }
}

struct half {
    template <typename T, std::enable_if_t<
        std::numeric_limits<T>::is_iec559 && sizeof(T) * CHAR_BIT == 32
    >*...>
    half(T value) {
        int ex;

        std::uint_fast32_t f = detail::bit_cast<std::uint32_t>(value);

        auto f_s = f & 0x80000000u;
        auto f_e = f & 0x7F800000u;
        auto f_m = f & 0x007FFFFFu;

        std::uint_fast16_t h_s = f_s >> 16u;
        std::uint_fast16_t h_e;
        std::uint_fast16_t h_m;

        auto e = detail::as_signed(f_e >> 23u) - 127;
        if (e < -15 - 10) {
            // Zero or a subnormal that will underflow to zero regardless of
            // mantissa.

            h_e = 0;
            h_m = 0;

            ex = FP_UNDERFLOW;
        } else if (e <= -15) {
            // Subnormal that might or might not underflow depending on
            // mantissa.

            h_e = 0;

            auto wide_m = 0x800000u | f_m;

            if (wide_m & ((1 << (-1 - e)) - 1)) {
                ex = FP_UNDERFLOW;
            }

            auto m = wide_m >> detail::as_unsigned(-14 - e);

            // Round to even.
            if (((m & 0x3FFFu) ^ 0x1000u) | (wide_m & 0x7FFu)) {
                m += 0x1000u;
            }

            h_m = m >> 13u;
        }

        half::bits = h_s | (h_e + h_m);

        std::feraiseexcept(ex);
    }

    half()
    = default;

private:
    std::int16_t bits;
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
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(x) ^ 0b10000000'00000000u
        )
    );
}

inline
auto fabs(half x)
-> half
{
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(x) & 0b01111111'11111111u
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
{ return detail::bit_cast<std::uint16_t>(x) & 0b10000000'00000000u; }

inline
auto copysign(half x, half y)
-> half
{
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(y) & 0b10000000'00000000u |
            detail::bit_cast<std::uint16_t>(x) & 0b01111111'11111111u
        )
    );
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
