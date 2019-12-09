#ifndef HALFFLOAT_DETAIL_INCLUDED_HPP
#define HALFFLOAT_DETAIL_INCLUDED_HPP

#include <cstdint>
#include <cfenv>
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
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(x) ^ 0x80u
        )
    );
}

inline
auto fabs(half x)
-> half
{
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(x) & 0x7Fu
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
{ return detail::bit_cast<std::uint16_t>(x) & 0x80u; }

inline
auto copysign(half x, half y)
-> half
{
    return detail::bit_cast<half>(
        static_cast<std::uint16_t>(
            detail::bit_cast<std::uint16_t>(y) & 0x80u |
            detail::bit_cast<std::uint16_t>(x) & 0x7Fu
        )
    );
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
