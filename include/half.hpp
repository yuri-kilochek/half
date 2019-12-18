#ifndef HALF_DETAIL_INCLUDED
#define HALF_DETAIL_INCLUDED

#include <cstdint>
#include <cfenv>
#include <cstddef>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>

namespace half_detail {
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


    template <std::size_t Radix, std::size_t Width, typename T>
    struct is_iec559
    : std::bool_constant<std::numeric_limits<T>::is_iec559 &&
                         std::numeric_limits<T>::radix == Radix &&
                         sizeof(T) * CHAR_BIT == Width>
    {};

    template <std::size_t Radix, std::size_t Width, typename T>
    constexpr
    auto is_iec559_v = is_iec559<Radix, Width, T>::value;


    template <typename T, typename = void>
    struct iec559_traits;

    template <typename T>
    struct iec559_traits<T, is_iec559_v<2, 32, T>> {
        using bits_type = std::uint32_t;
        using bits_fast_type = std::uint_fast32_t;

        constexpr
        std::uint_fast8_t exponent_width = 8;
    };

    template <typename T>
    struct iec559_traits<T, is_iec559_v<2, 64, T>> {
        using bits_type = std::uint64_t;
        using bits_fast_type = std::uint_fast64_t;

        constexpr
        std::uint_fast8_t exponent_width = 11;
    };

    template <typename T>
    using bits_type = typename iec559_traits<T>::bits_type;

    template <typename T>
    using bits_fast_type = typename iec559_traits<T>::bits_fast_type;

    template <typename T>
    constexpr
    auto width = sizeof(bits_type<T>) * CHAR_BIT;

    template <typename T>
    constexpr
    auto exponent_width = iec559_traits<T>::exponent_width;

    template <typename T>
    constexpr
    auto mantissa_width = width<T> - exponent_width<T> - 1;
}

struct half {
    template <typename T, std::enable_if_t<detail::is_iec559_v<2, 32, T>>*...>
    half(T value) {
        int excepts = 0;

        std::uint_fast32_t f = detail::bit_cast<std::uint32_t>(value);

        auto f_s = f & 0x80000000u;
        auto f_e = f & 0x7F800000u;
        auto f_m = f & 0x007FFFFFu;

        std::uint_fast16_t h = f_s >> 16;

        bool done;
        {
            // zeros and definitely underflowing subnormals
            bool do_now = f_e < 0x33000000u;

            bool underflowed = f & 0x7FFFFFFFu;
            excepts |= -(do_now & underflowed) & FE_UNDERFLOW;

            done = do_now;
        }
        {
            // subnormals
            bool do_now = !done & (f_e <= 0x38000000u);

            int e = f_e >> 23;
            auto wide_m = 0x00800000u | f_m;
            auto m = wide_m >> ((113 - e) & -do_now);
            m += -std::uint_fast32_t(((m & 0x00003FFFu) != 0x00001000u) | !!(wide_m & 0x000007FFu)) & 0x00001000u;
            std::uint_fast16_t h_m = m >> 13;
            h |= -std::uint_fast16_t(do_now) & h_m;

            bool underflowed = ((std::uint_fast32_t{1} << ((126 - e) & -do_now)) - 1u) & wide_m;
            excepts |= -(do_now & underflowed) & FE_UNDERFLOW;

            done |= do_now;
        }
        {
            // normals
            bool do_now = !done & (f_e < 0x47800000u);

            std::uint_fast16_t h_e = (f_e - 0x38000000u) >> 13;
            auto m = f_m;
            m += -std::uint_fast32_t((m & 0x00003FFFu) != 0x00001000u) & 0x00001000u;
            std::uint_fast16_t h_m = m >> 13;
            auto h_em = h_e + h_m;
            h |= -std::uint_fast16_t(do_now) & h_em;

            bool overflowed = h_em == 0x7C00u;
            excepts |= -(do_now & overflowed) & FE_OVERFLOW;

            done |= do_now;
        }
        {
            // definitely overflowing normals
            bool do_now = !done & (f_e < 0x7F800000u);

            h |= -std::uint_fast16_t(do_now) & 0x7C00u;

            excepts |= -do_now & FE_OVERFLOW;

            done |= do_now;
        }
        {
            // infinities
            bool do_now = !done & !f_m;

            h |= -std::uint_fast16_t(do_now) & 0x7C00u;

            done |= do_now;
        }
        {
            // nans
            bool do_now = !done;

            std::uint_fast16_t h_m = f_m >> 13;
            h_m |= !h_m;
            auto h_em = 0x7C00u | h_m;
            h |= -std::uint_fast16_t(do_now) & h_em;
        }

        half::bits = h;

        std::feraiseexcept(excepts);
    }

    half()
    = default;

private:
    std::uint16_t bits;
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
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(x) ^ 0x80u
        )
    );
}


inline
auto fabs(half x)
-> half
{
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(x) & 0x7Fu
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
{ return half_detail::bit_cast<std::uint16_t>(x) & 0x80u; }


inline
auto copysign(half x, half y)
-> half
{
    return half_detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            half_detail::bit_cast<std::uint16_t>(y) & 0x80u |
            half_detail::bit_cast<std::uint16_t>(x) & 0x7Fu
        )
    );
}

#endif
