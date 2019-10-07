#ifndef HALF_DETAIL_INCLUDE_GUARD
#define HALF_DETAIL_INCLUDE_GUARD

#include <cstdint>
#include <cstddef>
#include <limits>
#include <climits>
#include <cstring>
#include <type_traits>

class half {
    std::uint16_t bits_;

    // TODO: C++2a: Replace with concept.
    template <typename T, std::size_t BitCount>
    static constexpr
    bool const is_ieee754_binary_v
        = std::numeric_limits<T>::is_iec559 && sizeof(T) * CHAR_BIT == BitCount;

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

    static constexpr 
    std::uint_fast16_t const sign_mask = 0b10000000'00000000;

public:
    //template <typename Float32, std::enable_if_t<is_ieee754_binary_v<Float32, 32>>*...>
    //half(Float32 float32) {
    //    std::uint_fast32_t bits32 = bit_cast<std::uint32_t>(float32);


    //}

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
        y.bits_ = x.bits_ ^ sign_mask;
        return y;
    }

    friend
    auto abs(half x)
    -> half
    {
        half y;
        y.bits_ = x.bits_ & ~sign_mask;
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
